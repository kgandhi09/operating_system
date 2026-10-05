#include "sensor.h"
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <jk/runtime.hpp>
#include <set>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace jk {
[[noreturn]] void test_abandon_metadata(const std::string &);
}
using Sample = jkbuf_demo_SensorSample;
using namespace std::chrono_literals;
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(std::string("check failed: ") + #x + " line " + \
                               std::to_string(__LINE__));                      \
  } while (0)
template <class F> void rejects(F f) {
  bool failed = false;
  try {
    f();
  } catch (const jk::Error &) {
    failed = true;
  }
  CHECK(failed);
}
std::string domain(const char *suffix) {
  return "test-" + std::to_string(getpid()) + "-" + suffix;
}
void wait_child(pid_t pid, int expected = 0) {
  int status;
  CHECK(waitpid(pid, &status, 0) == pid);
  CHECK(WIFEXITED(status));
  CHECK(WEXITSTATUS(status) == expected);
}
void byte_write(int fd) {
  char c = 'x';
  CHECK(write(fd, &c, 1) == 1);
}
void byte_read(int fd) {
  char c;
  CHECK(read(fd, &c, 1) == 1);
}
void loans_and_bounds() {
  auto d = domain("loans");
  jk::LocalMaster master(d);
  jk::Node producer(d, "producer"), a(d, "a"), b(d, "b");
  auto pub = producer.publisher<Sample>("/sensor");
  auto sa = a.subscriber<Sample>("/sensor");
  auto sb = b.subscriber<Sample>("/sensor");
  auto raw = jk::shared_memory_transport(d, "mismatch");
  rejects([&] {
    raw->topic("/sensor", {jk::MessageTraits<Sample>::schema + 1,
                           sizeof(Sample), alignof(Sample)});
  });
  rejects([&] { jk::Node duplicate(d, "a"); });
  rejects([&] { auto duplicate = a.subscriber<Sample>("/sensor"); });
  rejects([&] { jk::LocalMaster duplicate(d); });
  rejects([&] { jk::LocalMaster::cleanup(d); });
  auto loan = pub.loan();
  CHECK(loan);
  CHECK((*loan)->f_frame.length == 0);
  (*loan)->f_sequence = 42;
  (*loan)->f_value = 7.5;
  auto slot = loan->slot();
  auto generation = loan->generation();
  pub.publish(std::move(*loan));
  rejects([&] { (void)**loan; });
  auto la = sa.take();
  auto lb = sb.take();
  CHECK(la && lb);
  CHECK(la->slot() == slot && lb->slot() == slot);
  CHECK(la->generation() == generation && lb->generation() == generation);
  CHECK((*la)->f_sequence == 42 && (*lb)->f_value == 7.5);
  // Each Node maps independently; equal allocation identity, different
  // addresses.
  CHECK(&**la != &**lb);
  CHECK(!sa.take());
  CHECK(!sb.take(5ms));
  std::vector<jk::WriteLoan<Sample>> retained;
  for (unsigned i = 1; i < jk::pool_slots; ++i) {
    auto l = pub.loan();
    CHECK(l);
    retained.push_back(std::move(*l));
  }
  CHECK(!pub.loan());
  la.reset();
  CHECK(!pub.loan());
  lb.reset();
  auto reused = pub.loan();
  CHECK(reused);
  CHECK(reused->slot() == slot);
  CHECK(reused->generation() > generation);
  retained.clear();
  reused.reset();
  auto invalid = pub.loan();
  CHECK(invalid);
  (*invalid)->f_frame.length = 33;
  rejects([&] { pub.publish(std::move(*invalid)); });
  CHECK(!sa.take());
  invalid.reset();
  auto other = producer.publisher<Sample>("/other");
  auto wrong = pub.loan();
  CHECK(wrong);
  rejects([&] { other.publish(std::move(*wrong)); });
}
void ordering_and_retention() {
  auto d = domain("ordering");
  jk::LocalMaster master(d);
  jk::Node p(d, "p"), q(d, "q"), r(d, "r");
  auto pub = p.publisher<Sample>("/topic");
  // No subscriber: immediately reclaimed, no retained history.
  for (unsigned i = 0; i < 100; ++i) {
    auto l = pub.loan();
    CHECK(l);
    pub.publish(std::move(*l));
  }
  auto sub = q.subscriber<Sample>("/topic");
  CHECK(!sub.take());
  auto sub2 = r.subscriber<Sample>("/topic");
  for (unsigned i = 0; i < jk::pool_slots; ++i) {
    auto l = pub.loan();
    CHECK(l);
    (*l)->f_sequence = i;
    pub.publish(std::move(*l));
  }
  CHECK(!pub.loan());
  for (unsigned i = 0; i < jk::pool_slots; ++i) {
    auto l = sub.take();
    CHECK(l);
    CHECK((*l)->f_sequence == i);
  }
  CHECK(
      !pub.loan()); // Other subscriber's pending deliveries retain every slot.
  for (unsigned i = 0; i < jk::pool_slots; ++i) {
    auto l = sub2.take();
    CHECK(l);
    CHECK((*l)->f_sequence == i);
  }
  CHECK(pub.loan());
}
void independent_processes() {
  auto d = domain("processes");
  jk::LocalMaster master(d);
  int ready[2], go[2];
  CHECK(pipe(ready) == 0);
  CHECK(pipe(go) == 0);
  std::vector<pid_t> children;
  for (unsigned i = 0; i < 2; ++i) {
    auto pid = fork();
    CHECK(pid >= 0);
    if (!pid) {
      alarm(15);
      try {
        jk::Node node(d, "reader-" + std::to_string(i));
        auto sub = node.subscriber<Sample>("/topic");
        byte_write(ready[1]);
        byte_read(go[0]);
        auto l = sub.take(2s);
        CHECK(l);
        CHECK((*l)->f_sequence == 123);
        CHECK(l->slot() == 0 && l->generation() == 1);
        byte_write(ready[1]);
        byte_read(go[0]);
        CHECK((*l)->f_sequence ==
              123); // Still valid while parent exhausts pool.
        _exit(
            0); // Deliberately abandon reader loan to exercise process reaping.
      } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        _exit(1);
      }
    }
    children.push_back(pid);
  }
  byte_read(ready[0]);
  byte_read(ready[0]);
  jk::Node node(d, "publisher");
  auto pub = node.publisher<Sample>("/topic");
  auto l = pub.loan();
  CHECK(l);
  (*l)->f_sequence = 123;
  pub.publish(std::move(*l));
  byte_write(go[1]);
  byte_write(go[1]);
  byte_read(ready[0]);
  byte_read(ready[0]);
  std::vector<jk::WriteLoan<Sample>> held;
  for (unsigned i = 1; i < jk::pool_slots; ++i) {
    auto x = pub.loan();
    CHECK(x);
    held.push_back(std::move(*x));
  }
  CHECK(!pub.loan());
  byte_write(go[1]);
  byte_write(go[1]);
  for (auto pid : children)
    wait_child(pid);
  master.reap();
  CHECK(pub.loan());
  for (int fd : {ready[0], ready[1], go[0], go[1]})
    close(fd);
}
void newest_delivery() {
  auto d = domain("newest");
  jk::LocalMaster master(d);
  jk::Node p(d, "p"), ordered_node(d, "ordered"), latest_node(d, "latest");
  auto pub = p.publisher<Sample>("Sensor Telemetry");
  rejects([&] { p.publisher<Sample>("invalid\ttopic"); });
  rejects([&] { latest_node.subscriber<Sample>("Sensor Telemetry",
                    static_cast<jk::SubscriptionPolicy>(99)); });
  auto latest = latest_node.subscriber<Sample>(
      "Sensor Telemetry", jk::SubscriptionPolicy::POLL_NEWEST);
  auto send = [&](unsigned sequence) {
    auto loan = pub.loan();
    CHECK(loan);
    (*loan)->f_sequence = sequence;
    pub.publish(std::move(*loan));
  };
  std::optional<jk::ReadLoan<Sample>> held;
  {
    auto ordered = ordered_node.subscriber<Sample>("Sensor Telemetry");
    for (unsigned i = 0; i < jk::pool_slots; ++i)
      send(i);
    CHECK(!pub.loan()); // Ordered delivery still retains the full burst.
    auto message = latest.take();
    CHECK(message);
    held.emplace(std::move(*message));
    CHECK((*held)->f_sequence == jk::pool_slots - 1);
    CHECK(!latest.take());
    for (unsigned i = 0; i < jk::pool_slots; ++i) {
      auto message = ordered.take();
      CHECK(message && (*message)->f_sequence == i);
    }
  }
  // An idle latest-only reader retains one pending sample, not every sample.
  // The separately held read loan must never be overwritten.
  for (unsigned i = jk::pool_slots; i < 1000; ++i)
    send(i);
  CHECK((*held)->f_sequence == jk::pool_slots - 1);
  auto newest = latest.take();
  CHECK(newest && (*newest)->f_sequence == 999);
  CHECK(!latest.take());
  held.reset();
  newest.reset();
  {
    auto temporary = ordered_node.subscriber<Sample>(
        "Sensor Telemetry", jk::SubscriptionPolicy::POLL_NEWEST);
    send(1000);
  }
  // Unsubscription clears the policy too, even if this node index is reused.
  auto ordered_again = ordered_node.subscriber<Sample>("Sensor Telemetry");
  send(1001);
  send(1002);
  auto first = ordered_again.take();
  auto second = ordered_again.take();
  CHECK(first && second);
  CHECK((*first)->f_sequence == 1001 && (*second)->f_sequence == 1002);
}
void crashed_writer_and_live_reader() {
  auto d = domain("crash");
  jk::LocalMaster master(d);
  int ready[2];
  CHECK(pipe(ready) == 0);
  auto pid = fork();
  CHECK(pid >= 0);
  if (!pid) {
    alarm(15);
    try {
      jk::Node n(d, "writer");
      auto p = n.publisher<Sample>("/topic");
      std::vector<jk::WriteLoan<Sample>> held;
      for (unsigned i = 0; i < jk::pool_slots; ++i) {
        auto l = p.loan();
        CHECK(l);
        held.push_back(std::move(*l));
      }
      byte_write(ready[1]);
      for (;;)
        pause();
    } catch (...) {
      _exit(1);
    }
  }
  byte_read(ready[0]);
  jk::Node n(d, "parent");
  auto p = n.publisher<Sample>("/topic");
  CHECK(!p.loan());
  master.reap();
  CHECK(!p.loan()); // Living but stalled owner cannot be reclaimed.
  CHECK(kill(pid, SIGKILL) == 0);
  int status;
  CHECK(waitpid(pid, &status, 0) == pid);
  CHECK(WIFSIGNALED(status));
  master.reap();
  CHECK(p.loan());
  close(ready[0]);
  close(ready[1]);
}
void concurrent_publishers() {
  auto d = domain("concurrent");
  jk::LocalMaster master(d);
  jk::Node reader(d, "reader");
  auto sub = reader.subscriber<Sample>("/topic");
  std::vector<pid_t> children;
  for (unsigned worker = 0; worker < 2; ++worker) {
    auto pid = fork();
    CHECK(pid >= 0);
    if (!pid) {
      alarm(15);
      try {
        jk::Node node(d, "writer-" + std::to_string(worker));
        auto pub = node.publisher<Sample>("/topic");
        for (unsigned i = 0; i < 200; ++i) {
          for (;;) {
            auto loan = pub.loan();
            if (!loan) {
              std::this_thread::sleep_for(1ms);
              continue;
            }
            (*loan)->f_sequence = worker * 200 + i;
            pub.publish(std::move(*loan));
            break;
          }
        }
        _exit(0);
      } catch (...) {
        _exit(1);
      }
    }
    children.push_back(pid);
  }
  std::set<std::uint64_t> values;
  std::uint64_t sequence = 0;
  for (unsigned i = 0; i < 400; ++i) {
    auto loan = sub.take(5s);
    CHECK(loan);
    CHECK(loan->sequence() > sequence);
    sequence = loan->sequence();
    CHECK(values.insert((*loan)->f_sequence).second);
  }
  CHECK(values.size() == 400 && *values.begin() == 0 &&
        *values.rbegin() == 399);
  for (auto pid : children)
    wait_child(pid);
  master.reap();
}
void master_restart_and_mutex_death() {
  auto d = domain("restart");
  auto master = std::make_unique<jk::LocalMaster>(d);
  jk::Node old(d, "old");
  auto pub = old.publisher<Sample>("/topic");
  auto retained = pub.loan();
  CHECK(retained);
  (*retained)->f_sequence = 99;
  master.reset();
  rejects([&] { pub.loan(); });
  master = std::make_unique<jk::LocalMaster>(d);
  jk::Node fresh(d, "fresh");
  auto freshpub = fresh.publisher<Sample>("/topic");
  CHECK(freshpub.loan());
  CHECK((*retained)->f_sequence ==
        99); // Old mapping remains allocated, never reused by new master.
  rejects([&] { pub.publish(std::move(*retained)); });
  auto pid = fork();
  CHECK(pid >= 0);
  if (!pid)
    jk::test_abandon_metadata(d);
  wait_child(pid, 77);
  rejects([&] { freshpub.loan(); });
  rejects([&] { master->reap(); });
}
int main() {
  alarm(30);
  try {
    loans_and_bounds();
    ordering_and_retention();
    newest_delivery();
    independent_processes();
    crashed_writer_and_live_reader();
    concurrent_publishers();
    master_restart_and_mutex_death();
    std::cout << "PASS: fan-out, independent mappings/processes, retention, "
                 "exhaustion, bounds, ordering, crash recovery, restart, "
                 "robust mutex failure\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
