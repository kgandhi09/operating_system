# Class-based node applications

Derive from `jk::NodeApp` and run the class from an executable's `main()`.
Launch each executable separately: each has its own process, node registration,
and event loop. `RunNode` runs in the current process; it does not fork.
Start `nodemaster` first and give communicating nodes the same domain.

```cpp
#include <jk/node_app.hpp>
#include "telemetry.h" // Generated from your telemetry.jkbuf schema.

using Bms = jkbuf_robot_Bms;
using DriveCmd = jkbuf_robot_DriveCmd;

class TelemetryNode : public jk::NodeApp {
    Bms bms_{};

public:
    explicit TelemetryNode(const Options &options)
        : NodeApp("TelemetryNode", options) {
        SetNotificationTimeoutMs(2);
        Subscribe(&TelemetryNode::HandleBms, "RX Bms TEL",
                  jk::SubscriptionPolicy::POLL_NEWEST);
        ProvidesSHM<DriveCmd>("TX Drive Cmd TEL");
    }

    void HandleBms(jk::MsgPtr<Bms> &message) {
        bms_ = *message; // Keep a local UI snapshot; the read loan is released.
    }

    void SendDriveCommand(float speed) {
        auto message = PrepareMessage<DriveCmd>("TX Drive Cmd TEL");
        if (!message)
            return; // Pool full: choose your application's retry/drop behavior.
        message->f_speed = speed;
        TransmitMessage(message);
    }

    void Update() override {
        // Poll window events and render one frame using bms_.
        // Call SendDriveCommand() when appropriate for your application.
        // SetState(FINISHED) when the window closes, or FAILED on error.
    }

    void Finalize() override {
        // Release UI resources. Also use RAII for constructor-failure cleanup.
    }
};

int main(int argc, char **argv) {
    return jk::RunNode<TelemetryNode>(argc, argv);
}
```

This sketch assumes your schema defines `robot.Bms` and `robot.DriveCmd`, with
an `f32 speed` field in `DriveCmd`. Generated fields have the `f_` prefix.
Link the executable to its message target and `jk::jk_nodeinfra`:

```cmake
add_executable(telemetry_node telemetry_node.cpp)
target_link_libraries(telemetry_node PRIVATE messages jk::jk_nodeinfra)
```

## Running the included examples

Build nodeinfra as described in [the main README](../README.md), then run these
in three terminals, in order:

```sh
build/nodeinfra/runtime/nodemaster demo
build/nodeinfra/examples/jk_class_subscriber --domain demo
build/nodeinfra/examples/jk_class_publisher --domain demo
```

The publisher sends ten samples and both node applications finish. Source:
[publisher](../examples/class_publisher.cpp),
[subscriber](../examples/class_subscriber.cpp).
`--domain` defaults to `default`; `--help` prints usage. Node names must be
unique within the domain. Ctrl-C and SIGTERM request orderly shutdown.

## Callback and delivery behavior

Register subscriptions and publishers in your constructor, before dispatch
starts. Callbacks, `Update()`, and `Finalize()` execute serially on the thread
calling `RunNode`, normally the main/UI thread. Each round calls at most one
callback per subscription, then `Update()`, including when no messages arrive.
`SetNotificationTimeoutMs(2)` sets the polling interval; it is not a separate
two-millisecond blocking wait for each subscription. Long callbacks or a
blocking `Update()` delay the whole loop and signal handling.

| Policy | Behavior |
| --- | --- |
| `BLOCK_NEXT` (default) | Deliver the next pending sample in publication order; preserve pending samples and apply pool backpressure. |
| `POLL_NEWEST` | Keep only the latest pending sample for this subscriber; superseded pending samples are released during publication. |

`BLOCK_NEXT` describes ordered delivery; the application loop still polls
nonblocking so other subscriptions and rendering continue. A newest reader
does not discard an ordered reader's pending samples or invalidate anyone's
active read loans. There is no replay of samples published before subscription.

Newest delivery is **per topic, not per `bot_id` inside a message**. For a UI
showing multiple robots, use per-robot topics or an ordered receiver that
maintains a latest-value cache per robot; otherwise one robot's sample can
supersede another's before your callback filters it.

## Loans, shutdown, and current limits

- `MsgPtr<T>` is a move-only read loan exposing `const T`. Do not retain a raw
  pointer after the callback's loan is released. Copy a UI snapshot, or move
  the loan into your own storage when retention is intentional.
- `PrepareMessage<T>()` returns a managed object used with `auto message`,
  `if (message)`, and `message->field`; it is not a raw pointer. Destruction
  cancels an unpublished loan. Successful `TransmitMessage(message)` consumes
  the loan; accessing or transmitting it again throws.
- `Run()` is single-use and calls `Finalize()` once on normal finish, requested
  stop, or an exception in dispatch/`Update()`. Failures return a nonzero exit
  code. Constructor failures require ordinary C++ RAII; `Finalize()` cannot
  run on an object that did not finish construction.
- `SpinOnce()` supports embedding one dispatch round in an existing loop; that
  loop owns shutdown/finalization. The API is single-threaded.
- `ProvidesSHM<T>(topic)` has no per-topic capacity argument. All topics still
  share 32 slots of up to 64 KiB each. Active loans or slow ordered subscribers
  can exhaust that pool. There are at most 32 nodes and 32 topics per domain.
- Topic names accept printable ASCII including spaces, up to 127 bytes.
  Node names are limited to 63 bytes and cannot contain spaces.
- The shared-memory layout is now version 2. Rebuild and restart the master
  and all clients together; old and new binaries cannot share one domain.
- This API uses the existing local shared-memory transport. Remote networking,
  automatic reconnection, and process supervision remain separate future work.
