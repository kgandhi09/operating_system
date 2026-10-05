#include "sensor.h"
#include <iostream>
#include <jk/node_app.hpp>

using Sample = jkbuf_demo_SensorSample;
class SensorNode : public jk::NodeApp {
  std::uint64_t sequence_ = 0;

public:
  explicit SensorNode(const Options &options) : NodeApp("sensor", options) {
    ProvidesSHM<Sample>("Sensor Telemetry");
    SetNotificationTimeoutMs(20);
  }
  void Update() override {
    auto message = PrepareMessage<Sample>("Sensor Telemetry");
    if (!message)
      return; // Shared pool is full; retry on a later iteration.
    message->f_sequence = sequence_;
    message->f_value = sequence_ * 0.5;
    TransmitMessage(message);
    std::cout << "PUBLISHED " << sequence_ << '\n' << std::flush;
    if (++sequence_ == 10)
      SetState(FINISHED);
  }
};

int main(int argc, char **argv) { return jk::RunNode<SensorNode>(argc, argv); }
