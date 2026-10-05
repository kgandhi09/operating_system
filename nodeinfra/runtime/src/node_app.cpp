#include <csignal>
#include <iostream>
#include <jk/node_app.hpp>
#include <thread>

namespace jk {
NodeApp::NodeApp(const std::string &name) : NodeApp(name, Options{}) {}
NodeApp::NodeApp(const std::string &name, const Options &options)
    : node_(options.domain, name) {}

void NodeApp::RequireRegistration() const {
  if (started_)
    throw Error("register publishers and subscriptions before running the node");
}
void NodeApp::SetNotificationTimeoutMs(std::uint32_t milliseconds) {
  if (milliseconds > 1000)
    throw Error("notification timeout must be between 0 and 1000 ms");
  poll_interval_ = std::chrono::milliseconds(milliseconds);
}
void NodeApp::SpinOnce() {
  if (in_dispatch_)
    throw Error("recursive node dispatch is not supported");
  started_ = true;
  if (state_ != RUNNING)
    return;
  in_dispatch_ = true;
  try {
    for (auto &subscription : subscriptions_) {
      subscription->Dispatch();
      if (state_ != RUNNING)
        break;
    }
    if (state_ == RUNNING)
      Update();
  } catch (...) {
    in_dispatch_ = false;
    throw;
  }
  in_dispatch_ = false;
}
int NodeApp::Run(const std::function<bool()> &stop_requested) {
  if (run_called_ || in_dispatch_)
    throw Error("node Run may only be called once and outside callbacks");
  run_called_ = true;
  started_ = true;
  auto fail = [this](const char *message) {
    state_ = FAILED;
    if (error_.empty())
      error_ = message;
  };
  try {
    while (state_ == RUNNING) {
      if (stop_requested && stop_requested()) {
        state_ = FINISHED;
        break;
      }
      auto start = std::chrono::steady_clock::now();
      SpinOnce();
      if (state_ == RUNNING)
        std::this_thread::sleep_until(start + poll_interval_);
    }
  } catch (const std::exception &e) {
    fail(e.what());
  } catch (...) {
    fail("unknown exception in node callback or Update");
  }
  bool failed = state_ == FAILED;
  try {
    Finalize();
  } catch (const std::exception &e) {
    fail(e.what());
    failed = true;
  } catch (...) {
    fail("unknown exception in node Finalize");
    failed = true;
  }
  // Finalize cannot accidentally erase an earlier failure or restart a node.
  state_ = (failed || state_ == FAILED) ? FAILED : FINISHED;
  return state_ == FAILED ? 1 : 0;
}

namespace {
volatile std::sig_atomic_t stop_signal = 0;
void request_stop(int) { stop_signal = 1; }
class SignalHandlers {
  struct sigaction old_int_{}, old_term_{};

public:
  SignalHandlers() {
    stop_signal = 0;
    struct sigaction action {};
    action.sa_handler = request_stop;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, &old_int_) != 0)
      throw Error("cannot install SIGINT handler");
    if (sigaction(SIGTERM, &action, &old_term_) != 0) {
      sigaction(SIGINT, &old_int_, nullptr);
      throw Error("cannot install SIGTERM handler");
    }
  }
  ~SignalHandlers() {
    sigaction(SIGTERM, &old_term_, nullptr);
    sigaction(SIGINT, &old_int_, nullptr);
  }
};
} // namespace
int RunNode(int argc, char **argv, const NodeFactory &factory) {
  try {
    NodeApp::Options options;
    for (int i = 1; i < argc; ++i) {
      std::string argument = argv[i];
      if (argument == "--help") {
        std::cout << "usage: " << argv[0] << " [--domain NAME]\n";
        return 0;
      }
      if (argument == "--domain" && i + 1 < argc)
        options.domain = argv[++i];
      else
        throw Error("invalid arguments (use --help)");
    }
    SignalHandlers signals;
    auto app = factory(options);
    if (!app)
      throw Error("node factory returned null");
    int result = app->Run([] { return stop_signal != 0; });
    if (result != 0)
      std::cerr << "node: " << (app->LastError().empty() ? "failed" : app->LastError()) << '\n';
    return result;
  } catch (const std::exception &e) {
    std::cerr << "node: " << e.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "node: unknown exception during startup\n";
    return 1;
  }
}
} // namespace jk
