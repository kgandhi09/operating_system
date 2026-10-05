# Write a node from scratch

A minimal project with one publisher and one subscriber, each running in its
own process. Requires Linux, CMake 3.20+, and a C++20 compiler.

```text
basic_nodes/
├── CMakeLists.txt
├── publisher.cpp
├── subscriber.cpp
└── messages/
    ├── CMakeLists.txt
    └── temperature.jkbuf
```

## Build and run

From the JK OS repository root:

```sh
cmake -S nodeinfra/examples/basic_nodes -B build/basic_nodes
cmake --build build/basic_nodes -j4
```

Run these in three separate terminals, in order:

```sh
# Terminal 1: shared-memory domain owner.
build/basic_nodes/nodeinfra/runtime/nodemaster demo

# Terminal 2: wait for READY before starting the publisher.
build/basic_nodes/temperature_subscriber --domain demo

# Terminal 3: publish ten simulated temperature readings.
build/basic_nodes/temperature_publisher --domain demo
```

Both nodes exit after ten messages. Stop the master with Ctrl-C. The nodes must
use the same domain and topic; messages sent before subscription are not replayed.

## Make your own node

1. Define fields in [messages/temperature.jkbuf](messages/temperature.jkbuf).
   `tutorial.Temperature` generates `jkbuf_tutorial_Temperature`; fields become
   `f_sequence` and `f_celsius`.
2. Add the schema with `build_jkbuf(NAME temperature_msg MSG_FILES temperature.jkbuf)`
   in [messages/CMakeLists.txt](messages/CMakeLists.txt). Headers generate during
   the build, and linking `messages` supplies their include paths.
3. Derive a class from `jk::NodeApp`. In its constructor, register publishers
   with `ProvidesSHM<T>(topic)` and callbacks with `Subscribe(&Class::Handler, topic)`.
4. In `Update()`, prepare a message, check the loan, fill its fields, and call
   `TransmitMessage(message)`. Receive through `void Handler(jk::MsgPtr<T>& message)`.
   See [publisher.cpp](publisher.cpp) and [subscriber.cpp](subscriber.cpp).
5. Call `jk::RunNode<YourClass>(argc, argv)` from `main()`. Add an executable in
   [CMakeLists.txt](CMakeLists.txt) and link `messages` and `jk::jk_nodeinfra`.

Callbacks and `Update()` run on the same thread. Use `BLOCK_NEXT` for ordered
events, or `POLL_NEWEST` for a display that only needs the latest pending sample.
Call `SetState(FINISHED)` to exit; override `Finalize()` for shutdown work.
Ctrl-C also requests graceful shutdown.

To copy this project elsewhere, configure it with
`-DNODEINFRA_SOURCE_DIR=/absolute/path/to/jk_os/nodeinfra`.
See the [NodeApp guide](../../runtime/README.md) for ownership and runtime limits.
