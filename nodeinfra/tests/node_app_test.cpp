#include "sensor.h"
#include <iostream>
#include <jk/node_app.hpp>
#include <thread>
#include <unistd.h>

using Sample = jkbuf_demo_SensorSample;
static_assert(std::is_same_v<decltype(std::declval<jk::MsgPtr<Sample>>().operator->()),
                             const Sample *>);
#define CHECK(condition)                                                        \
  do {                                                                         \
    if (!(condition))                                                          \
      throw std::runtime_error("check failed: " #condition);                    \
  } while (0)
template <class F> void rejects(F action) {
  bool rejected = false;
  try { action(); } catch (const jk::Error &) { rejected = true; }
  CHECK(rejected);
}

class App : public jk::NodeApp {
  std::thread::id thread_ = std::this_thread::get_id();

public:
  unsigned updates = 0, finalized = 0, stop_after = 0;
  bool fail_callback = false, fail_update = false, fail_finalize = false;
  std::vector<std::uint64_t> ordered, latest;
  using NodeApp::PrepareMessage;
  using NodeApp::ProvidesSHM;
  using NodeApp::TransmitMessage;
  App(const std::string &name, const std::string &domain) : NodeApp(name, Options{domain}) {
    SetNotificationTimeoutMs(0);
    Subscribe(&App::OnOrdered, "Ordered Topic");
    Subscribe(&App::OnLatest, "Latest Topic", jk::SubscriptionPolicy::POLL_NEWEST);
    ProvidesSHM<Sample>("Commands");
  }
  void OnOrdered(jk::MsgPtr<Sample> &message) {
    CHECK(thread_ == std::this_thread::get_id());
    if (fail_callback)
      throw jk::Error("callback failed");
    ordered.push_back(message->f_sequence);
  }
  void OnLatest(jk::MsgPtr<Sample> &message) { latest.push_back(message->f_sequence); }
  void Update() override {
    CHECK(thread_ == std::this_thread::get_id());
    ++updates;
    if (fail_update)
      throw jk::Error("update failed");
    if (stop_after && updates >= stop_after)
      SetState(FINISHED);
  }
  void Finalize() override {
    ++finalized;
    if (fail_finalize)
      throw jk::Error("finalize failed");
    SetState(FINISHED); // Must not hide a previous exception.
  }
};

void send(jk::Publisher<Sample> &publisher, unsigned sequence) {
  auto message = publisher.loan();
  CHECK(message);
  (*message)->f_sequence = sequence;
  publisher.publish(std::move(*message));
}

int main() {
  alarm(20);
  try {
    auto domain = "nodeapp-" + std::to_string(getpid());
    jk::LocalMaster master(domain);
    jk::Node producer(domain, "producer");
    auto ordered = producer.publisher<Sample>("Ordered Topic");
    auto latest = producer.publisher<Sample>("Latest Topic");
    {
      App app("app", domain), other("other", domain);
      rejects([&] { app.ProvidesSHM<Sample>("Commands"); });
      rejects([&] { app.PrepareMessage<Sample>("Missing"); });
      rejects([&] { app.PrepareMessage<std::uint64_t>("Commands"); });
      rejects([&] { app.SetNotificationTimeoutMs(1001); });
      auto message = app.PrepareMessage<Sample>("Commands");
      CHECK(message);
      message->f_sequence = 42;
      rejects([&] { other.TransmitMessage(message); });
      auto moved = std::move(message);
      CHECK(!message && moved);
      rejects([&] { (void)message->f_sequence; });
      moved->f_frame.length = 33;
      rejects([&] { app.TransmitMessage(moved); });
      moved->f_frame.length = 0;
      app.TransmitMessage(moved);
      CHECK(!moved);
      rejects([&] { app.TransmitMessage(moved); });
      {
        std::vector<jk::PreparedMessage<Sample>> held;
        for (unsigned i = 0; i < jk::pool_slots; ++i) {
          auto loan = app.PrepareMessage<Sample>("Commands");
          CHECK(loan);
          held.push_back(std::move(loan));
        }
        CHECK(!app.PrepareMessage<Sample>("Commands"));
      }
      CHECK(app.PrepareMessage<Sample>("Commands")); // Destruction cancels loans.
      for (unsigned i = 0; i < 5; ++i) {
        send(ordered, i);
        send(latest, i);
      }
      app.SpinOnce();
      CHECK(app.ordered == std::vector<std::uint64_t>{0});
      CHECK(app.latest == std::vector<std::uint64_t>{4});
      CHECK(app.updates == 1);
      rejects([&] { app.ProvidesSHM<Sample>("Late Registration"); });
      for (unsigned i = 0; i < 4; ++i)
        app.SpinOnce();
      CHECK(app.ordered.size() == 5 && app.ordered.back() == 4);
      app.SpinOnce();
      CHECK(app.updates == 6); // Idle subscriptions never prevent UI updates.
      app.stop_after = 9;
      CHECK(app.Run() == 0 && app.finalized == 1 && app.updates == 9);
      rejects([&] { app.Run(); });
      CHECK(app.finalized == 1);
    }
    {
      App app("throw-callback", domain);
      app.fail_callback = true;
      send(ordered, 10);
      CHECK(app.Run() == 1 && app.GetState() == jk::NodeApp::FAILED);
      CHECK(app.LastError() == "callback failed");
      CHECK(app.finalized == 1 && app.updates == 0);
    }
    {
      App app("throw-update", domain);
      app.fail_update = true;
      CHECK(app.Run() == 1 && app.finalized == 1);
      CHECK(app.LastError() == "update failed");
    }
    {
      App app("throw-finalize", domain);
      app.fail_finalize = true;
      app.SetState(jk::NodeApp::FINISHED);
      CHECK(app.Run() == 1 && app.finalized == 1 && app.updates == 0);
    }
    {
      App app("stopped", domain);
      CHECK(app.Run([] { return true; }) == 0);
      CHECK(app.finalized == 1 && app.updates == 0);
    }
    std::cout << "PASS: callbacks, newest/ordered delivery, idle updates, loans, shutdown, exceptions\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
