#include "temperature.h"
#include <iostream>
#include <jk/node_app.hpp>

using Temperature = jkbuf_tutorial_Temperature;

class TemperatureSubscriber : public jk::NodeApp {
  unsigned received_ = 0;

public:
  explicit TemperatureSubscriber(const Options &options)
      : NodeApp("TemperatureSubscriber", options) {
    Subscribe(&TemperatureSubscriber::HandleTemperature, "Temperature",
              jk::SubscriptionPolicy::BLOCK_NEXT);
    std::cout << "READY TemperatureSubscriber\n" << std::flush;
  }

  void HandleTemperature(jk::MsgPtr<Temperature> &message) {
    std::cout << "RECEIVED " << message->f_sequence
              << " celsius=" << message->f_celsius << '\n' << std::flush;
    if (++received_ == 10)
      SetState(FINISHED);
    // The read loan is released after this callback. Copy data if you need
    // a local snapshot for a later Update(); do not save the raw pointer.
  }

  void Update() override {
    // Add periodic work or UI rendering here. Runs even without messages.
  }
};

int main(int argc, char **argv) {
  return jk::RunNode<TemperatureSubscriber>(argc, argv);
}
