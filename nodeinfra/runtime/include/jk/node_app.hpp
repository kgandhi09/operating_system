#pragma once
#include <functional>
#include <jk/runtime.hpp>
#include <map>
#include <vector>

namespace jk {
// A move-only, read-only shared-memory loan. Valid until released/destroyed.
template <class T> using MsgPtr = ReadLoan<T>;

template <class T> class PreparedMessage {
  friend class NodeApp;
  std::shared_ptr<const char> owner_;
  std::shared_ptr<Publisher<T>> publisher_;
  std::optional<WriteLoan<T>> loan_;
  PreparedMessage(std::shared_ptr<const char> owner,
                  std::shared_ptr<Publisher<T>> publisher)
      : owner_(std::move(owner)), publisher_(std::move(publisher)),
        loan_(publisher_->loan()) {}

public:
  PreparedMessage(const PreparedMessage &) = delete;
  PreparedMessage &operator=(const PreparedMessage &) = delete;
  PreparedMessage(PreparedMessage &&) noexcept = default;
  PreparedMessage &operator=(PreparedMessage &&) = delete;
  explicit operator bool() const { return publisher_ && loan_.has_value(); }
  T *operator->() {
    if (!*this)
      throw Error("message loan is empty or already transmitted");
    return loan_->operator->();
  }
  T &operator*() { return *operator->(); }
};

// One instance is normally created by each node executable. Registration and
// all callbacks/Update/Finalize run on the calling thread; this is not a
// thread-safe executor. Construct fresh instances after fork, never before it.
class NodeApp {
public:
  struct Options {
    std::string domain = "default";
  };
  enum State { RUNNING, FINISHED, FAILED };

  explicit NodeApp(const std::string &name);
  NodeApp(const std::string &name, const Options &options);
  virtual ~NodeApp() = default;
  NodeApp(const NodeApp &) = delete;
  NodeApp &operator=(const NodeApp &) = delete;

  virtual void Update() {}
  virtual void Finalize() {}
  void SetState(State state) { state_ = state; }
  State GetState() const { return state_; }
  const std::string &LastError() const { return error_; }
  // Maximum idle interval between polling rounds, not a per-topic wait.
  void SetNotificationTimeoutMs(std::uint32_t milliseconds);
  // One nonblocking polling round: at most one callback per subscription,
  // then Update(), even when there are no messages.
  void SpinOnce();
  // Calls Finalize exactly once on normal stop or callback/Update failure.
  // The stop predicate is checked between rounds. Run may only be called once.
  int Run(const std::function<bool()> &stop_requested = {});

protected:
  template <class App, class T>
  void Subscribe(void (App::*handler)(MsgPtr<T> &), const std::string &topic,
                 SubscriptionPolicy policy = SubscriptionPolicy::BLOCK_NEXT) {
    RequireRegistration();
    auto *app = dynamic_cast<App *>(this);
    if (!app || !handler)
      throw Error("subscription handler must belong to this node class");
    subscriptions_.push_back(std::make_unique<Subscription<T>>(
        node_.subscriber<T>(topic, policy),
        [app, handler](MsgPtr<T> &message) { (app->*handler)(message); }));
  }

  template <class T> void ProvidesSHM(const std::string &topic) {
    RequireRegistration();
    if (publishers_.contains(topic))
      throw Error("publisher already provided: " + topic);
    publishers_.emplace(topic, std::make_shared<Publication<T>>(
                                   node_.publisher<T>(topic)));
  }

  template <class T> PreparedMessage<T> PrepareMessage(const std::string &topic) {
    auto found = publishers_.find(topic);
    if (found == publishers_.end())
      throw Error("publisher was not provided: " + topic);
    auto publisher = std::dynamic_pointer_cast<Publication<T>>(found->second);
    if (!publisher)
      throw Error("publisher message type mismatch: " + topic);
    return PreparedMessage<T>(owner_, publisher->endpoint);
  }

  template <class T> void TransmitMessage(PreparedMessage<T> &message) {
    if (message.owner_ != owner_)
      throw Error("message belongs to another node");
    if (!message)
      throw Error("message loan is empty or already transmitted");
    message.publisher_->publish(std::move(*message.loan_));
    message.loan_.reset();
  }

private:
  struct SubscriptionBase {
    virtual ~SubscriptionBase() = default;
    virtual void Dispatch() = 0;
  };
  template <class T> struct Subscription : SubscriptionBase {
    Subscriber<T> endpoint;
    std::function<void(MsgPtr<T> &)> callback;
    Subscription(Subscriber<T> subscriber,
                 std::function<void(MsgPtr<T> &)> handler)
        : endpoint(std::move(subscriber)), callback(std::move(handler)) {}
    void Dispatch() override {
      if (auto message = endpoint.take())
        callback(*message);
    }
  };
  struct PublicationBase {
    virtual ~PublicationBase() = default;
  };
  template <class T> struct Publication : PublicationBase {
    std::shared_ptr<Publisher<T>> endpoint;
    explicit Publication(Publisher<T> publisher)
        : endpoint(std::make_shared<Publisher<T>>(std::move(publisher))) {}
  };
  void RequireRegistration() const;
  Node node_;
  std::shared_ptr<const char> owner_ = std::make_shared<const char>(0);
  std::vector<std::unique_ptr<SubscriptionBase>> subscriptions_;
  std::map<std::string, std::shared_ptr<PublicationBase>> publishers_;
  State state_ = RUNNING;
  std::chrono::milliseconds poll_interval_{2};
  bool started_ = false, in_dispatch_ = false, run_called_ = false;
  std::string error_;
};

using NodeFactory = std::function<std::unique_ptr<NodeApp>(const NodeApp::Options &)>;
// CLI: executable [--domain NAME]. Installs/restores SIGINT/SIGTERM handlers;
// call from main(), once per process. Exceptions become a nonzero exit status.
int RunNode(int argc, char **argv, const NodeFactory &factory);
template <class App> int RunNode(int argc, char **argv) {
  static_assert(std::is_base_of_v<NodeApp, App>);
  return RunNode(argc, argv, [](const NodeApp::Options &options) {
    return std::make_unique<App>(options);
  });
}
} // namespace jk
