#include "sensor.h"
#include <iostream>
#include <jk/node_app.hpp>

using Sample = jkbuf_demo_SensorSample;
class DisplayNode : public jk::NodeApp {
  unsigned received_ = 0;

public:
  explicit DisplayNode(const Options &options) : NodeApp("display", options) {
    Subscribe(&DisplayNode::HandleSample, "Sensor Telemetry",
              jk::SubscriptionPolicy::BLOCK_NEXT);
    SetNotificationTimeoutMs(2);
    std::cout << "READY display\n" << std::flush;
  }
  void HandleSample(jk::MsgPtr<Sample> &message) {
    std::cout << "RECEIVED " << message->f_sequence
              << " value=" << message->f_value << '\n' << std::flush;
    if (++received_ == 10)
      SetState(FINISHED);
  }
  void Update() override {
    // Render one UI frame here, even when no telemetry has arrived.
    // For a latest-value display use POLL_NEWEST and cache data in the handler.
  }
  void Finalize() override { std::cout << "FINALIZED display\n" << std::flush; }
};

int main(int argc, char **argv) { return jk::RunNode<DisplayNode>(argc, argv); }
