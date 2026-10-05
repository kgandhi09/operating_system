# nodeinfra: jkbuf and local shared-memory nodes

The first local implementation is written in **C++20**, with no ROS dependency.
It includes the `jkbuf` RIDL compiler, `nodemaster`, a transport interface,
and typed publisher/subscriber APIs. Networking and JKLink controller
integration are the next layer; this implementation opens no network sockets.

## Build and test

From the JK OS repository root:

```sh
cmake -S nodeinfra -B build/nodeinfra -DCMAKE_BUILD_TYPE=Debug
cmake --build build/nodeinfra -j4
ctest --test-dir build/nodeinfra --output-on-failure
```

Use a fresh build directory, including after renaming the source directory.
The previously copied `nodeinfra/build`
contains a cache from the original source location and is not used.

Requirements: Linux, a C++20 compiler, CMake 3.20+, and pthreads. Tests also
use Python 3; the original lexer tests run when GoogleTest is installed.
Nothing is downloaded. `-DBUILD_TESTING=OFF` removes test dependencies.
`-DJK_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF` builds only the compiler,
master, and runtime library. Host compiler/toolchain settings and install
prefix are respected.

## Run the minimal node apps

Run these in separate terminals, as the **same OS user**, starting the
subscriber before the publisher:

```sh
# Terminal 1: manages the local "demo" shared-memory domain.
build/nodeinfra/runtime/nodemaster demo

# Terminal 2: subscribes and receives ten messages.
build/nodeinfra/examples/jk_subscriber demo dashboard 10

# Optional terminal 3: another independent subscriber to the same payloads.
build/nodeinfra/examples/jk_subscriber demo logger 10

# Final terminal: constructs and publishes ten messages in shared memory.
build/nodeinfra/examples/jk_publisher demo 10
```

The subscriber prints `READY` after registration. Each publication and
reception prints a slot and generation: both subscribers see the **same
allocation identity**, even though their virtual addresses can differ.
Stop the master with Ctrl-C. After a killed/crashed master, remove its stale
domain before restarting:

```sh
build/nodeinfra/runtime/nodemaster --cleanup demo
```

Cleanup refuses an active master. Old clients must exit and reconnect to
the new domain; the new master never reuses memory still mapped by old
clients. A small, zero-length lifecycle lock object remains in `/dev/shm`
until reboot to serialize creation and cleanup safely.

## Compile a schema

```ridl
namespace robot {
    message Reading {
        u64 sequence;
        f64 temperature;
        string<32> sensor;
        vector<f32, 16> samples;
    }
}
```

```sh
build/nodeinfra/idl/jkbuf -o reading.h reading.ridl
# Includes are relative to the including file, then searched through -I paths.
build/nodeinfra/idl/jkbuf -I schemas -o reading.h reading.ridl
```

The generated header contains pointer-free C99-compatible message structs,
initializers, bounds validators, schema IDs, and C++ `jk::MessageTraits`.
The compiler, node API, transport, and master themselves are C++.
`robot.Reading` becomes `jkbuf_robot_Reading`; fields have an `f_` prefix.
Strings and vectors contain `length` and inline `data`; strings count bytes
and do not require or provide a NUL terminator. Fixed arrays have `data`.
All generated storage is inline and bounded.

The compiler supports namespaces, includes, forward references, primitive
and composite types, and comments. It rejects unresolved/recursive types,
duplicate declarations or fields, invalid bounds, generated-name collisions,
and messages exceeding 64 KiB. Diagnostics include source locations where
available. See [the RIDL specification](idl/grammar/ridl_spec.md).

## C++ node API

```cpp
#include <jk/runtime.hpp>
#include "reading.h"

jk::Node node("demo", "temperature-source");
auto publisher = node.publisher<jkbuf_robot_Reading>("/robot/temperature");
auto loan = publisher.loan();
if (loan) {
    (*loan)->f_sequence = 1;
    (*loan)->f_temperature = 24.5;
    publisher.publish(std::move(*loan));
}
```

In another process:

```cpp
jk::Node node("demo", "display");
auto subscriber = node.subscriber<jkbuf_robot_Reading>("/robot/temperature");
auto loan = subscriber.take(std::chrono::seconds(1));
if (loan) {
    const auto &message = **loan;
    // Read message.f_temperature directly from the shared allocation.
} // Destruction releases this reader's retention.
```

Loans are move-only. Successful publication consumes the writable loan;
never retain or use its raw pointer afterward. An unpublished loan is
cancelled on destruction. A read loan exposes only const data and retains
the allocation until destruction. Loans keep their transport/session alive.
Do not inherit Nodes or loans across `fork`; create fresh Nodes in children.

`loan()` returns an empty optional when the shared pool is full. `take()`
returns an empty optional on timeout; schema mismatches, invalid messages,
master loss, and protocol/lifecycle errors throw `jk::Error`. There is no
implicit retry or message replay.

### Install and use from another CMake project

```sh
cmake --install build/nodeinfra --prefix /tmp/nodeinfra-install
```

```cmake
find_package(jk_nodeinfra CONFIG REQUIRED)
target_link_libraries(my_node PRIVATE jk::jk_nodeinfra)
```

Configure the consuming project with
`-DCMAKE_PREFIX_PATH=/tmp/nodeinfra-install`. Compile generated headers with
the installed include directory. The exported target carries C++20 and
thread dependencies. `jkbuf` and `nodemaster` install into `bin`.

For cross builds, first build a native `jkbuf`, then provide a CMake
cross-toolchain and `-DJKBUF_HOST_EXECUTABLE=/path/to/native/jkbuf` so schema
generation runs on the build host. The target build also produces a target
`jkbuf` executable for generating schemas directly on that machine.

## Near-zero-copy behavior and initial limits

- A publisher loans an existing shared slot. The runtime clears its message
  storage once, and the publisher constructs the message there directly.
- Publication updates routing/retention metadata under a process-shared
  mutex. **No payload is copied on publish or once per local subscriber.**
- Readers map the same backing object and receive a slot/generation handle.
  The nodemaster never receives and resends payload bytes.
- Releasing the last pending/active reader makes a published slot reusable.
  Separate publications still require separate allocations; no content
  deduplication is implied.
- One domain has **32 slots of 64 KiB**, **32 nodes**, and **32 topic names**.
  Topic registrations last until domain restart. One subscriber per
  node/topic is supported; multiple publishers per topic are supported.
- Topics require exact schema ID, size, and alignment agreement. Delivery
  order follows publication commit order; there is no retained history.
- Slow readers and pending deliveries retain slots. Exhaustion is explicit;
  live reader memory is never overwritten to make room for a new message.
- This first version uses a robust process-shared mutex and polling waits
  capped at 2 ms between checks. It is not lock-free, allocation-free, or
  hard real-time. No throughput/latency target is claimed.
- Domains are restricted to one effective OS user with mode-0600 shared
  memory. Participants are cooperating processes, not mutually sandboxed
  applications: a malicious participant can modify its writable mapping.
  API-level const access is not hardware-enforced read-only protection.
- Layouts are native to the local ABI. Generated size checks reject
  unsupported layouts. These structs are **not a portable network format**.
  TCP framing, serialization, machine identity, remote permissions,
  controller routing, and Python bindings remain future work.

See [OWNERSHIP.md](OWNERSHIP.md) for commit, crash, and restart invariants.
The tests cover generated C/C++ headers, separate-process fan-out, allocation
identity, slow readers, pool exhaustion, concurrent publishers, abnormal
process exit, robust-mutex owner death, master restart, and stale cleanup.
