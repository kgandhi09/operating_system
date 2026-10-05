# JKLink: distributed node infrastructure for jk_os

Status: **local C++ node infrastructure implemented; networking/controller pending** (2026-10-05).
jk_os is running on an x86 machine and the Samsung tablet. These are the
initial machines for developing and validating JKLink.

## Goal and decisions

JKLink provides one reusable communication infrastructure for robotics,
desktop features, and future applications. Applications participate as
nodes, managed locally by a nodemaster. A controller connects nodemasters
across machines so their nodes can communicate.

- **Our own node infrastructure:** JKLink defines its own node API,
  lifecycle, discovery, messaging, and permissions. We will not use ROS,
  ROS 2, or their node infrastructure, and will not build a ROS bridge.
- **TCP for all JKLink networking:** no UDP discovery, multicast, QUIC,
  or required VPN overlay. Connections between machines use TLS over TCP.
- **One nodemaster per machine:** local communication continues without
  the controller or an internet connection.
- **Controller-mediated cross-machine communication first:** the
  controller initially routes application traffic as well as management
  traffic between nodemasters.
- **Required identity fields:** machine role, machine name, and username,
  supplemented by persistent identifiers and authenticated credentials.
- **Portable controller:** start it on the x86 workstation, then move its
  configuration and persistent state to a dedicated management machine.
- **Architecture and language independence:** the wire format must work
  across x86_64 and aarch64 and support C, C++, Python, and future clients.

The local milestone is implemented in [distributed_arch](distributed_arch/README.md):
`jkbuf`, `nodemaster`, and a near-zero-copy C++ shared-memory publisher/subscriber
transport. See its README for working commands and current limits. The
cross-machine architecture below remains planned; its wire format, machine
identity, trust, and controller APIs still need implementation.

## Architecture

```text
                      JKLink controller
               registry / permissions / routing
                      |                 |
                 TCP + TLS         TCP + TLS
                      |                 |
           x86 workstation         Samsung tablet
           local nodemaster        local nodemaster
             |    |    |              |         |
        compute  file  clipboard   dashboard   clipboard
          node   node     node        node        node

       Each node uses the same JKLink application API.
       A robot joins through its own local nodemaster in the same way.
```

### Nodes

A node performs application work: reading a sensor, controlling an
actuator, processing data, displaying telemetry, sharing a clipboard, or
transferring a file. It may be a standalone process or part of a larger
application. Each logical node registers a distinct identity and its
offered topics, services, or transfer capabilities with its nodemaster.

Nodes use the JKLink API to publish, subscribe, make requests, respond,
and transfer data. They do not need to manage controller connections or
know whether a destination is local or remote.

Application behavior belongs in nodes. A file node handles filesystem
access and receiving-user approval; a clipboard node handles the selected
user's clipboard and sharing preferences. These are applications of the
same infrastructure, not separate networking systems.

### Nodemaster

One nodemaster runs on each participating machine. It:

- Registers and authenticates local node sessions and tracks their health.
- Maintains the local directory of topics, services, and capabilities.
- Routes local messages directly between local nodes.
- Enforces local permissions and bounded resource usage.
- Connects to the configured controller, advertises permitted local
  capabilities, and forwards authorized cross-machine traffic.
- Reports machine and node availability and restores registrations and
  subscriptions after reconnecting.

Local nodes now use the in-house shared-memory transport. The nodemaster
creates the per-user domain and monitors process lifetime; nodes update shared
registration and routing metadata under a robust mutex. Publishers write
into loaned slots and subscribers read the same allocations. Local delivery
opens no network sockets and remains independent of the future controller.
Domains currently admit cooperating processes with the same effective OS UID;
stronger per-node permissions and remote user identity binding remain future
work. All future JKLink network transport remains TCP-based.

Managing registration and health does not automatically grant permission
to execute programs. Optional process launching or restarting can be
added later with explicit local configuration.

### Controller

The controller is the JKLink management server. It:

- Enrolls and authenticates machines and their nodemasters.
- Maintains the machine registry and a permission-filtered directory of
  available nodes, topics, services, and capabilities.
- Enforces cross-machine access rules, including rules based on machine
  role, user, node, and operation.
- Routes publications, requests, responses, and transfers between
  nodemasters in the first implementation.
- Tracks heartbeats, current sessions, and last-seen information.
- Provides local administration for enrollment, inspection, role changes,
  and revocation.

Running the controller is a service capability, not a requirement to
change a machine's role. The x86 PC can retain its workstation role and
run both its own nodemaster and the controller.

## Routing and discovery

Local traffic:

```text
publisher -> local nodemaster -> local subscriber
```

Cross-machine traffic in the first implementation:

```text
publisher -> local nodemaster -> controller -> remote nodemaster -> subscriber
```

Nodemasters make outbound TCP connections to an explicitly configured
controller address. Both LAN and internet deployments use this model;
the controller must be reachable from each machine. No inbound connection
to a client machine is required for this initial routing model.

Local discovery comes from registration with the nodemaster. Remote
discovery comes from the controller's authorized directory, not multicast
scouting. Subscriptions and routing information must be withdrawn when
their sessions expire, so disconnected nodes are not advertised as live.

Later, the controller may authorize direct TCP connections between
nodemasters where reachable, preserving the same application API. This is
an optional optimization, not a dependency of the first version.

## Identity and trust

Identity distinguishes a machine from the users and nodes running on it.

| Field          | Meaning                                                                     |
| -------------- | --------------------------------------------------------------------------- |
| Machine ID     | Persistent unique identifier, independent of names and network addresses    |
| Machine role   | Workstation, tablet, robot, server, or an administrator-defined role        |
| Machine name   | Human-readable name, such as `warehouse-robot-01`                           |
| Username       | Authenticated OS user or service account owning a node session              |
| Node ID        | Identifier for a logical node, scoped to its machine                        |
| Node name/type | Readable name and application function, such as `front-camera` / `camera`   |
| Session ID     | Distinguishes a live connection from earlier connections of the same node   |
| Credentials    | Prove the machine or local user's identity; names alone do not authenticate |

Several users may run nodes on one machine. The username therefore belongs
to the authenticated node session, not a single global machine owner.
Forwarded messages preserve machine, user, and node identity; a node may
not impersonate another node by supplying different source fields.
The nodemaster vouches for its locally authenticated sessions, and the
controller validates that their machine identity matches its connection.

Machine role supports discovery and policy, but is not proof of trust.
A machine cannot gain robot-control permissions by announcing `robot` as
its role. Enrollment and authorized administration establish its role.

Examples of policy:

- A tablet dashboard can subscribe to telemetry from approved robots.
- Only designated control nodes may send motion commands to a robot.
- A workstation and tablet may exchange files for an authorized user.
- Clipboard sharing is opt-in and scoped to permitted user sessions.
- Machines in different organizations or fleets are isolated unless an
  explicit policy allows communication.

TLS protects inter-machine TCP connections and authenticates enrolled
machines. Credential provisioning, controller trust, renewal, and
revocation must be defined before exposing a deployment to other machines.
There is no default trust-everyone network mode.

Enrollment policy is still to be selected: short-lived single-use
administrator-issued credentials, approval of pending requests, or factory
provisioning for robots. Revocation must terminate active access as well
as prevent new sessions.

## Shared communication model

### Publish/subscribe

Nodes publish named topics, and authorized subscribers receive updates.
Readable topic paths can follow `/<machine>/<node>/<topic>`, for example
`/warehouse-robot-01/front-camera/frame`. Persistent IDs provide the
underlying identity so a rename does not create a different machine.
Name resolution and collision rules will be part of the protocol spec.

Discovery can filter by machine role or node capabilities. Wildcard
subscriptions, if supported, must still apply permissions to each matched
source; a broad subscription must not bypass access checks.

Uses include sensor readings, robot status, notifications, and clipboard
change events. Optional retained state can later provide the most recent
value to a new subscriber, with its timestamp and freshness made explicit.

### Request/reply

Nodes expose named services. Requests carry correlation IDs and deadlines;
responses identify the request they answer. Examples include querying
battery status, asking a planner for a route, or requesting a file.

Timeouts and disconnections must surface as errors. Reconnecting must not
silently replay a motion command or another operation with side effects.
Retry and deduplication behavior must be explicit; TCP delivery alone does
not establish that an application completed a request.

### Chunked transfers

Transfer sessions carry files and larger blobs with metadata, bounded
chunks, integrity checks, and explicit completion. Resume support can
follow once transfer-state persistence is implemented.

File nodes enforce allowed paths, overwrite policy, and recipient approval.
The transport itself does not grant remote filesystem access.

### Protocol and API foundation

Define a versioned, length-delimited message envelope with message type,
request/message identifiers, routing metadata, content type or schema
identifier, and payload length. Specify byte order, field encoding, size
limits, version compatibility, and error behavior independently of native
C/C++ memory layout.

Payloads can represent typed application messages or opaque binary data.
The exact control-message encoding and typed-schema system remain open.
The common API must hide routing details without hiding failures or
application delivery semantics.

The protocol must correctly handle partial reads/writes and multiple
frames in one TCP read. Validate lengths and limits before allocation,
reject malformed or incompatible messages, and bound connection counts,
queued data, subscriptions, and transfer resources.

The intended application interfaces are a C API usable from C++, plus
Python bindings or an interoperable Python client. Exact library and
executable names will be finalized during implementation.

## Delivery, performance, and availability

- TCP provides ordered, reliable bytes within a live connection. It does
  not provide persistence, application acknowledgments, or exactly-once
  processing across reconnects.
- Use bounded queues and explicit backpressure. Latest-only topics may
  replace unsent queued samples; they cannot skip bytes already queued in
  TCP. Define queue overflow behavior per operation.
- Put bulk transfers on separate TCP connections from latency-sensitive
  messaging, with fair scheduling and resource limits at each hop.
- On controller loss, local nodes continue communicating through their
  local nodemaster. Cross-machine traffic through the controller becomes
  unavailable and is reported as such.
- On reconnect, authenticate again and restore authorized registrations
  and subscriptions. Stale messages and commands are not replayed by
  default. Offline durable delivery is a separate future feature.
- A nodemaster failure interrupts its local node connections. Clients
  need bounded reconnect behavior and explicit session restoration.
- TCP, the nodemaster, and controller routing do not guarantee real-time
  deadlines. Hard real-time and safety-critical loops must remain in an
  appropriate local execution path independent of controller availability.

The local shared-memory backend is implemented with bounded 64 KiB message
slots; larger sensor buffers and further performance work remain extensions.
Clock synchronization requirements and direct nodemaster links are future
work. These must preserve the public API and the TCP-only networking decision.

## Applications built on the node infrastructure

| Application          | Nodes and communication                                                       |
| -------------------- | ----------------------------------------------------------------------------- |
| Robot sensing        | Camera, lidar, and other sensor nodes publish data to processing nodes        |
| Robot control        | Authorized planning/control nodes send requests or commands to actuator nodes |
| Monitoring           | Tablet dashboard subscribes to robot telemetry and health                     |
| File sharing         | File nodes negotiate and transfer files, maps, models, and recordings         |
| Clipboard            | Per-user clipboard nodes exchange approved updates and avoid echo loops       |
| Notifications        | Nodes publish events for permitted desktop sessions                           |
| System visualization | `jk-viz` can later display machines, nodes, and their communication graph     |

Robotics and desktop applications use the same infrastructure and security
model. JKLink owns the node model; no ROS compatibility layer is planned.
Microcontroller participation is future work, subject to memory, TCP, and
authentication constraints.

## Controller deployment and administration

Start with one controller on the x86 workstation and a nodemaster on both
the workstation and Samsung tablet. The controller service is enabled
explicitly and integrated with jk_os boot and logging.

Keep persistent registry, policy, and credential state outside the
replaceable OS image. Live sessions are transient and must be re-established
after a restart; persisted last-seen data must not imply a machine is online.

Local administrative tools should support machine/session inspection,
enrollment, authorized name/role updates, and revocation. Remote application
nodes do not automatically receive administrative rights.

Moving to a dedicated management machine requires transferring the
controller configuration, database, and credentials securely, then updating
its address or retaining its DNS name and valid server identity. Enrollment
should not need to be repeated merely because the controller moved.
The initial design has one active controller; replication and failover are
future work.

C++ with OpenSSL and SQLite from the existing jk_os source tree is a
proposed implementation choice, not a constraint on the public protocol.
Deployment configuration must make the controller address and participation
visible and controllable, including for customer-owned machines.

## Implementation phases

1. **Protocol and identity:** specify framing, message types, identity and
   credential binding, naming, permissions, errors, limits, and node API.
   Define interoperability fixtures for architecture-independent messages.
2. **Local nodemaster (minimal implementation complete):** `distributed_arch`
   supplies per-user domains, local node/topic registration, process monitoring,
   near-zero-copy publish/subscribe, and example nodes. Fine-grained node
   authorization and richer directory/health tooling remain to be added.
3. **Controller and two-machine messaging:** implement persistent machine
   enrollment, TLS authentication, directory and subscription routing,
   cross-machine permissions, heartbeats, revocation, and reconnects.
   Connect the PC and tablet nodemasters through the PC's controller.
4. **Services and transfers:** add request/reply, deadlines, transfer
   sessions, checksums, separate bulk connections, and later resume support.
5. **Application nodes and integration:** build file, clipboard, dashboard,
   and robotics nodes; expand language interfaces and visualization.
6. **Deployment and optimization:** move the controller to dedicated
   hardware; evaluate direct nodemaster links, larger shared pools, and scale
   improvements without changing application APIs.

### First demonstrable milestone

A publisher on the x86 PC sends messages to a subscriber on the Samsung
tablet through their nodemasters and the controller. Machine role, machine
name, username, and node identity are visible and authenticated. Local
publish/subscribe on each machine continues when the controller stops;
remote availability recovers after it restarts and sessions reconnect.

### Verification

Test framing under fragmented and combined reads, malformed messages,
incompatible versions, oversized frames, and slow clients. Verify local
user isolation, enrollment failures, spoofed identities/roles, denied
subscriptions and commands, concurrent nodes, reconnects, stale-session
cleanup, controller restarts, and active-session revocation. Exercise
cross-architecture interoperability and confirm no JKLink UDP listeners or
discovery traffic are introduced.

## Remaining design questions

1. Enrollment workflow and local user credential provisioning.
2. Exact wire encoding, schema evolution, naming, and API signatures.
3. Default roles and operation-level permission rules.
4. Expected machine/node counts, sensor bandwidth, and latency targets.
5. Queue limits, retained-state policy, and transfer-resume requirements.
6. Whether and when to add process supervision and microcontroller clients.
7. Controller deployment address, credential lifecycle, and backup policy.

TCP-only networking, our own node infrastructure, the three-layer
architecture, and the required identity fields are settled requirements.
