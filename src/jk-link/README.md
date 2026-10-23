# jk-link

This machine's identity on the network, how it finds the other jk_os
machines, and which of them it trusts.

| Part | Where | What |
|---|---|---|
| `daemon/` | `/usr/sbin/jk-linkd` | one per machine, started by `init/S45jk-link` (root, then the user `jk-link`) |
| `cli/` | `/usr/bin/jk-link` | `jk-link`, `peers`, `pair`, `accept`, `unpair`, `add`, `visibility`, `events`, `gui` |
| `gui/` | `/usr/lib/jk_os/jk-link-gui` | the mesh console: `jk-link gui [<machine>]` |
| `lib/` | (static) | identity, beacon, addresses, store, JSON, control socket |

## Identity

On its first start jk-linkd makes an Ed25519 key pair, `identity.key` (0600)
in its state directory: `/data/etc/jk-link` on an installed system (kept
across updates and reinstalls of the OS image), `/var/lib/jk-link` on the
live system. The public key **is** the machine:

- machine ID `jk-7f3a-91c2`: the first 4 bytes of SHA-256(public key)
- fingerprint: the whole SHA-256, in groups of 4 hex digits

Names and addresses are labels only.

## Finding machines (discovery)

A signed beacon on UDP 7710 (`lib/beacon.h`): the sender's public key, TCP
port, name, arch, OS version and a sequence number, signed with its key.
Every 5 s, and at once (a query, which others answer) when an interface comes
up:

| Visibility | Beacons |
|---|---|
| `open` (default) | multicast on every interface with multicast (239.255.77.10, ff02::7710), and unicast to the addresses added by hand |
| `paired` | only unicast, to the paired machines where they were last seen and the addresses added by hand |
| `off` | none; paired machines can still connect to the addresses they know |

Beacons from others are always listened to. A machine is offline after 20 s
without a beacon or link traffic, or at once when it says goodbye (shutting
down, or switching visibility off). Where multicast doesn't get through
(some Wi-Fi networks), `jk-link add <ip[:port]>` names a machine to reach
directly. Beacons are only a hint: who a machine is gets proven again over
TLS.

## Knowing who it is (TLS and pairing)

jk-linkd listens on TCP 7710: TLS 1.3, both sides presenting a self-signed
certificate for their key. The key the other side proves must be the one its
beacon announced. Both sides then send a hello (name, arch, version, port),
and JSON lines.

Pairing: `jk-link pair <machine>` (or **Pair** in the console). Both machines
show the same 6-digit code, SHA-256 over both public keys: the user on each
side compares them. Something between the two (a machine pretending to be one
of them) would hold another key, so the codes would differ. Trust needs both:
the user who asked confirms the code, the other machine's user accepts the
request (`jk-link accept`, or **Review & accept**). Paired machines are kept in
`trust` (key, date, name then).

Between paired machines jk-linkd keeps a link up whenever they can reach each
other (the one opened by the smaller key, if both connect at once): a ping
every 3 s (liveness, round-trip time), and every 5 s the list of its own links,
which the console draws as the mesh. A paired machine that now calls itself
something else is flagged; its key is what counts. `jk-link unpair` makes both
sides forget each other.

Pairing, accepting, unpairing and settings need root or an administrator
(group `wheel`), checked on the control socket (`/run/jk-link/socket`, one
JSON request per line, `lib/ipc.h`). Anyone may look.

## State directory

| File | |
|---|---|
| `identity.key` | the private key |
| `trust` | the paired machines |
| `link.conf` | visibility, addresses added by hand |
| `cache` | where paired machines were last seen |

Ports: UDP 7710 (beacons), TCP 7710 (links).
