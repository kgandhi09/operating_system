# jk-link: connectivity between jk_os machines

Status: **concept, parked** (2026-10-04). To be picked up once jk_os runs on
several machines (PCs, the Galaxy Tab S7 FE, Jetson-class robot computers).

## Goal

Every jk_os machine can find and talk to other jk_os machines automatically,
as part of the OS:

- **on the same network** (zero configuration), and
- **over the internet**: every machine joins a private overlay network as soon
  as it is online, is visible to a master, and can discover other machines it
  is allowed to see.

Uses:

- **Desktop:** send and receive files and folders, shared clipboard,
  notifications, "send to device" in the file manager.
- **Robotics:** nodes on any machine and any arch publish and subscribe to
  topics, call services on other machines, transfer maps, logs and models.

## What it has to cover

| Need               | Desktop use                        | Robotics use                                                              |
| ------------------ | ---------------------------------- | ------------------------------------------------------------------------- |
| Discovery          | Which jk_os machines are around?   | Which robots and nodes are up?                                            |
| Identity & trust   | Only my machines can send me files | Only my fleet can drive my robot                                          |
| Pub/sub            | Clipboard, notifications, status   | Sensor streams, telemetry, commands                                       |
| Request/reply      | "Run this", "battery level?"       | Services and actions (plan a path, calibrate)                             |
| Bulk transfer      | Files and folders                  | Maps, logs, model files, recordings                                       |
| Quality of service | Hardly matters                     | Reliable vs latest-only, low latency, big messages (images, point clouds) |
| Cross-arch         | x86 PC ↔ aarch64 tablet            | Jetson ↔ PC ↔ microcontrollers                                            |

## Architecture

```
 apps:   jk-link CLI · desktop file sharing · robotics nodes (C/C++/Python)
            │
 API:    libjklink (C) + Python bindings      ← jk_os's own, stable API
            │
 daemon: jk-linkd (one per machine, started at boot)
          ├─ identity: machine key pair, created at first boot (jk-setup)
          ├─ trust: paired machines / fleet key, per-peer permissions
          ├─ discovery: mDNS/DNS-SD + multicast on the LAN,
          │             the master's peer list over the overlay network
          └─ router: topics, services, transfers
            │
 wire:   proven protocol: encrypted, multiplexed, UDP + TCP,
         shared memory between nodes on the same machine
            │
 network: the LAN directly, or the private overlay network (below)
```

### Discovery

- Each machine announces itself: name, ID (hash of its public key), arch,
  device type (PC, tablet, robot-HPC), what it offers.
- On a LAN: standard mDNS/DNS-SD (`_jklink._udp.local`) plus the wire
  protocol's multicast scouting.
- Where multicast is blocked (some Wi-Fi, corporate networks) and over the
  internet: the master's list of peers, or a manually given address.

### Identity and trust

- One key pair per machine (Ed25519), created at first boot.
- All traffic encrypted and authenticated, also on "our own" network.
- Trust modes:
    - **pairing** (desktops): the other machine shows a 6-digit code to confirm;
    - **fleet key** (robots): machines provisioned with the same fleet
      certificate trust each other;
    - **open** (lab bench): trust everything on the network, explicit opt-in,
      off by default.
- Per-peer permissions, e.g. the tablet may send files but not publish to
  `/robot/cmd_vel`.

### Data model

- **Topics:** `/<machine>/<node>/<topic>`, e.g. `/arm-01/lidar/scan`, with
  wildcards for subscribing.
- **Services:** `/arm-01/planner/plan_path`.
- **Queries / state:** latest value of X, optional storage of recent values
  (like ROS latched topics).
- **Transfers:** files and blobs, chunked, resumable, checksummed.
- **Serialization:** compact arch-neutral binary encoding; CBOR for untyped
  data, an IDL for typed messages so C++, Python and microcontrollers agree.

### Robotics specifics

- QoS: reliable / best-effort, keep last N, latest-only.
- Zero-copy shared memory between nodes on one machine (camera frames).
- Time sync between jk_os machines (PTP or chrony between peers).
- ROS 2 interop if the wire protocol is one ROS 2 supports.
- Real-time control loops stay on the local network; over the internet
  jk-link is for monitoring, fleet commands and transfers (tens of ms and up).

### Desktop integration

- `jk-link send <file> <machine>`, receive with confirmation.
- "Send to jk_os device" in the file manager, notifications, optional shared
  clipboard.
- `jk-viz` shows peers and the topic graph.

## Wire protocol: build or adopt

| Option              | Pros                                                                                                                                                                                  | Cons                                                                                                              |
| ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Fully custom        | Total control                                                                                                                                                                         | Years to get discovery, congestion control, security and QoS right; security mistakes likely; no interoperability |
| **Zenoh** (Eclipse) | Built for this: zero-config discovery, pub/sub + queries + storage, shared memory, efficient on Wi-Fi; official ROS 2 transport (`rmw_zenoh`); pure-C zenoh-pico for microcontrollers | Core is Rust: jk_os's offline source build would need a Rust toolchain                                            |
| Cyclone DDS         | Plain C, mature, ROS 2's classic middleware, strong QoS                                                                                                                               | Heavy, discovery scales poorly on Wi-Fi, complex security, no natural file transfer                               |
| MQTT / NATS         | Simple, popular                                                                                                                                                                       | Central broker, not peer-to-peer zero-config, weak for real-time robotics                                         |
| gRPC / QUIC         | Good request/reply and streaming                                                                                                                                                      | No discovery or pub/sub; we'd build most of it                                                                    |

**Leaning:** jk-link = jk_os's identity, trust, naming, daemon, tools and API,
with **Zenoh** underneath (fallback: Cyclone DDS if Rust in the build is not
acceptable). The wire protocol can be replaced later without breaking apps.

## Over the internet: the private overlay network

Every jk_os machine joins a private WireGuard mesh network as soon as it has
internet; a master (coordination server) knows all of them; machines connect
peer-to-peer; jk-link runs on top.

```
                 ┌──────────────────────────────┐
                 │ MASTER (coordination server) │  e.g. J.K. Robotics' cloud
                 │  · registry of all machines  │  server
                 │  · keys, names, VPN IPs      │
                 │  · access rules per fleet    │
                 │  · relay for hard NATs       │
                 └──────┬──────────┬────────────┘
        control only    │          │    (no data, unless relaying)
              ┌─────────┘          └─────────┐
        ┌─────┴─────┐  encrypted, direct ┌───┴───────┐
        │ jk_os PC  │◄══════════════════►│ jk_os     │
        │ 10.42.0.5 │   WireGuard tunnel │ tablet    │
        └───────────┘   (peer-to-peer)   │ 10.42.0.9 │
                                         └───────────┘
```

1. **First boot** (`jk-setup`): the machine makes its key pair and enrolls with
   the master (organisation/fleet code or QR), gets a stable private address
   and name (e.g. `tablet-01.jk`).
2. **Whenever online**, anywhere (home, office, a robot on 4G), it checks in:
   the master knows which machines exist and which are online.
3. **Direct encrypted tunnels** between machines, through NAT; the master
   relays when no direct path exists.
4. **jk-link over the overlay:** discovery from the master's peer list,
   filtered by access rules.

WireGuard is in the mainline kernel (one option); the coordination layer is
the work:

| Option                                         | Fit                                                                    | Notes                                                                                                                                                                  |
| ---------------------------------------------- | ---------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Tailscale client + self-hosted Headscale**   | Headscale = the master on our server; Tailscale client on each machine | Best NAT traversal and relays, mature; client is Go but ships static binaries (fits jk_os's prebuilt-binaries mechanism); no multicast, so discovery via the peer list |
| NetBird (self-hosted)                          | Management/signal/relay servers = master, kernel WireGuard             | BSD-3, web dashboard, Go                                                                                                                                               |
| Nebula                                         | Lighthouse = master, certificate identity                              | Simple, group firewall rules, Go static binary, fewer management tools                                                                                                 |
| ZeroTier (self-hosted controller)              | Virtual Ethernet: multicast works, LAN discovery unchanged             | Recent versions source-available (BSL): check for a product                                                                                                            |
| Our own (kernel WireGuard + jk control server) | Full control, integrated with jk-link identity                         | Most work: NAT traversal and relaying                                                                                                                                  |

**Leaning:** Headscale (master) + Tailscale client to start, wrapped in jk_os's
enrollment and jk-link identity; replaceable later.

### To decide carefully

1. **Consent and disclosure:** for our own machines this is fleet management;
   for customers' machines, connecting to our server must be disclosed and
   controllable (shown at setup, can be turned off or pointed at their own
   master).
2. **Isolation:** customer A's machines never see customer B's; separate
   networks or strict rules per organisation.
3. **Enrollment security:** org code/QR at setup, admin approval of new
   devices, or factory-provisioned keys for robots.
4. **Master availability:** existing tunnels survive an outage, but new
   machines can't join; a backed-up cloud server is fine to start.
5. **Latency:** control loops local; jk-link prefers direct LAN paths.

## Open questions

1. Scale: how many machines and nodes per network?
2. Networks: wired, Wi-Fi, both? Is multicast allowed where we deploy?
3. Security level: trusted lab or locked-down customer sites? Fleet keys for
   robots?
4. Heaviest robotics data (images, point clouds) and latency targets?
5. ROS 2 interop needed, or our own node framework?
6. Microcontrollers (STM32, ESP32) as nodes?
7. Rust in the jk_os build acceptable (for Zenoh)?
8. Only J.K. Robotics' own machines, or customers' too?
9. Master as a cloud server, a jk_os admin install, or both?
10. Enrollment: org code/QR, admin approval, or factory provisioning?

## Phases (once decided)

1. jk-linkd + identity + LAN discovery + `jk-link` CLI (list peers).
2. Pub/sub and services, C and Python APIs.
3. File transfer and desktop integration.
4. Overlay network: master, enrollment, peer discovery over the internet.
5. Robotics extras: QoS, shared memory, time sync, ROS 2 bridge,
   microcontroller nodes.
