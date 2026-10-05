#include "temperature.h"
#include <iostream>
#include <jk/node_app.hpp>

using Temperature = jkbuf_tutorial_Temperature;

class TemperaturePublisher : public jk::NodeApp {
  std::uint64_t sequence_ = 0;

public:
  explicit TemperaturePublisher(const Options &options)
      : NodeApp("TemperaturePublisher", options) {
    ProvidesSHM<Temperature>("Temperature");
    SetNotificationTimeoutMs(100);
  }

  void Update() override {
    auto message = PrepareMessage<Temperature>("Temperature");
    if (!message)
      return; // Pool full: retry on the next Update().

    message->f_sequence = sequence_;
    message->f_celsius = 20.0f + static_cast<float>(sequence_) * 0.5f;
    TransmitMessage(message); // Consumes the loan; do not access it afterward.

    std::cout << "PUBLISHED " << sequence_ << '\n' << std::flush;
    if (++sequence_ == 10)
      SetState(FINISHED);
  }
};

int main(int argc, char **argv) {
  return jk::RunNode<TemperaturePublisher>(argc, argv);
}
