# Local shared-memory ownership contract, version 2

This is the implemented contract for the C++ local backend. It narrows the
broader proposals in ARCHITECTURE.md for a minimal, reviewable first runtime.

## Domain and participant lifetime

A nodemaster owns one named POSIX shared-memory object per effective UID and
domain. It holds an exclusive lifecycle lock before creating that object.
A separate persistent lock inode serializes startup and stale-object cleanup;
cleanup cannot accidentally unlink a replacement master during a race.
The domain is mode 0600. Different users have different domain names and pools.

Initialization creates a robust, process-shared pthread mutex, initializes
tables, and publishes an ABI signature with a release store. A client checks
file size, ownership, permissions, signature, and the live master's lock
before accessing the mutex. Initializing domains produce a retryable-by-caller
connection error, not access to a half-initialized mutex.

A Node registers a name and a process identity consisting of Linux PID and
`/proc/PID/stat` start time. Node names are unique within a live domain.
The transport is held by shared ownership across its Node, endpoints, and
loans, so destroying a Node object does not prematurely release live loans.
After `fork`, inherited Nodes and loans must not be used. New child processes
register fresh Nodes; a fork does not register additional loan ownership.

## Fixed bounds

There are 32 participant entries, 32 topic entries, and 32 slots, each with
64-byte-aligned storage for at most 65536 bytes. Payload size/alignment and
schema ID are fixed when a topic is first registered. Topic names are retained
until domain restart, and incompatible re-registration is rejected.
Each topic records subscriber membership and a separate mask identifying
newest-only subscribers. Layout version 2 must match in the master and clients.

Each slot has a state, a generation, an owning writer index, a topic index,
a publication sequence, and separate 32-bit masks for pending deliveries and
active readers. Handles contain the slot index and generation, never a
shared process address. Each process derives its payload pointer from its
own mapping. The API's Buffer also caches that process-local pointer.

## State transitions

```text
FREE -> WRITABLE -> PUBLISHED -> FREE
          |                       ^
          +---- cancel -----------+
```

All mutable metadata transitions are serialized by the robust mutex.

1. **Loan:** select a free slot, increment its generation, set writer/topic,
   clear message storage once, and return exclusive writable access. There
   is no allocation or payload copy into a second publication buffer.
2. **Construct:** the application writes directly into the slot outside the
   metadata lock. It owns the loan exclusively and must serialize access to
   that loan itself. Publication validates generated bounds first.
3. **Commit:** under the lock, confirm slot generation/state and owner/topic,
   clear newest-only subscribers' pending bits on older publications of this
   topic (preserving all active-reader bits), reclaim any now-unretained slots,
   assign a monotonically increasing sequence, capture current subscriber
   bits, and switch to PUBLISHED. Unlock makes completed payload writes
   visible to subsequent readers acquiring the lock.
4. **Take:** a subscriber selects its oldest pending publication. Under the
   same lock it sets its active-reader bit and clears its pending bit. There
   is no unretained interval during queue-to-reader ownership transfer.
5. **Release:** clear that active-reader bit. A published slot becomes FREE
   only when both pending and reader masks are empty.
6. **Cancel:** an uncommitted writable loan returns directly to FREE when
   destroyed, after validating its owner and generation.

With no subscribers, commit immediately reclaims the slot. New subscribers
do not receive old publications. Unsubscribing releases pending deliveries
but preserves already-taken read loans until those loans are destroyed.
Generation and sequence overflow cause an error rather than wraparound.

A successful publish consumes its writable loan. Retaining a raw writable
pointer and accessing it afterward violates the ownership contract. Read
loans expose const views, but participants have writable shared mappings;
this first backend assumes cooperating applications in one user's domain.

## Backpressure and ordering

Pending deliveries and active readers share the bounded pool. A full pool
causes `loan()` to return no value. A slow ordered reader therefore applies
backpressure without losing its allocation. There is no eviction of active
reader loans, automatic retry,
unbounded offline queue, or automatic replay.

All matching active subscribers at commit are admitted using a single metadata
transaction. Taking messages scans slots by publication sequence. Per-topic
order follows commit order, including concurrent publishers. Application
completion and durable delivery are not guaranteed by publication.

`BLOCK_NEXT` preserves pending samples in commit order. `POLL_NEWEST` retains
only the newest pending sample for that subscriber. Coalescing changes pending
metadata, never the bytes or retention of an active read loan, and never the
pending bits of ordered subscribers. It happens during publication, so an idle
newest-only subscriber does not accumulate a backlog. A full pool can still
prevent loaning the next write slot; publication never steals a live allocation.
Unsubscription and participant cleanup clear both subscription masks.

## Death and recovery

The master periodically inspects participant process identities. Confirmed
termination (including zombies), or a changed start time for a reused PID,
allows removal of that process's subscriptions, queued deliveries, reader
retention, and abandoned writable loans. Ambiguous/unreadable process state
does not justify reclamation. A timeout or slow reader alone never does.

If a process dies while holding the metadata mutex, the next locker receives
`EOWNERDEAD`. The runtime marks the whole domain failed, makes the mutex
consistent so other processes can observe failure, and rejects new operations.
It does not guess which partially completed metadata writes are safe.
All clients must stop/reconnect after the master restarts. Existing mappings
stay allocated while any process still maps them, so this fail-closed path
never repurposes payload storage underneath existing readers.

Normal master shutdown marks the domain stopped and unlinks its name. Clients
report master loss. A killed master can leave a stale name; `nodemaster
--cleanup DOMAIN` acquires the lifecycle lock and refuses an active master
before unlinking stale data. A new master creates a new backing object.
Old clients never silently join it or replay old messages.

The mutex itself is not destroyed while other mappings may use it. POSIX
object lifetime retains the old backing storage until the last mapping/file
descriptor is released. This is also why an old loan can be destroyed safely
after a master restart.

## Tests establishing this contract

- Independent mappings and processes report identical slot/generation IDs.
- Two subscribers retain one allocation until the final read loan is released.
- Pending deliveries retain slots; unsubscribing or confirmed process exit
  releases the appropriate ownership without affecting live readers.
- Exhaustion, invalid bounds, schema mismatches, and duplicate nodes fail
  explicitly.
- Concurrent publishers deliver distinct messages in commit order.
- Mixed ordered/newest subscribers preserve ordered delivery while coalescing
  pending latest samples; active read loans remain unchanged across publication.
- A killed writer's unpublished loans are recovered; a living stalled writer
  is not reclaimed.
- Injected metadata-lock owner death fails the domain without reuse.
- Master restart gives new clients a new object while old loans remain in
  their old mapping; stale cleanup refuses a running master.
