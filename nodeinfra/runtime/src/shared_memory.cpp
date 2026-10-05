#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <jk/runtime.hpp>
#include <limits>
#include <pthread.h>
#include <sstream>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

namespace jk {
namespace {
constexpr std::uint64_t magic = 0x4a4b53484d303031ULL;
constexpr unsigned max_nodes = 32, max_topics = 32;
constexpr std::uint32_t layout_version = 2;
struct Process {
  std::int32_t pid;
  std::uint64_t start;
};
struct Participant {
  Process process;
  char name[64];
  bool used;
};
struct Topic {
  char name[128];
  std::uint64_t schema;
  std::uint32_t size, alignment, subscribers, newest_subscribers;
  bool used;
};
struct Slot {
  std::uint64_t generation, sequence;
  std::uint32_t state, writer, topic, pending, readers;
  alignas(64) std::byte payload[max_payload];
};
struct Shared {
  std::uint64_t signature;
  std::uint32_t version, size;
  Process master;
  pthread_mutex_t mutex;
  bool stopped, failed;
  std::uint64_t sequence;
  Participant nodes[max_nodes];
  Topic topics[max_topics];
  Slot slots[pool_slots];
};
std::string object_name(const std::string &domain) {
  if (domain.empty() || domain.size() > 48)
    throw Error("domain must contain 1..48 letters, digits, '-' or '_'");
  for (char c : domain)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_'))
      throw Error("invalid domain name");
  return "/jk-nodeinfra-" + std::to_string(geteuid()) + "-" + domain;
}
void check_name(const std::string &name, std::size_t limit,
                bool allow_spaces = false) {
  if (name.empty() || name.size() >= limit)
    throw Error("empty or oversized node/topic name");
  for (unsigned char c : name)
    if (c < (allow_spaces ? 32 : 33) || c > 126)
      throw Error(allow_spaces ? "topic names must contain printable ASCII"
                               : "node names must contain printable ASCII without spaces");
}
void system_error(const char *operation) {
  throw Error(std::string(operation) + ": " + std::strerror(errno));
}
// A PID plus Linux start time avoids reclaiming loans belonging to a reused
// PID. Missing /proc entries and zombies are dead; unreadable/ambiguous entries
// are unknown.
struct ProcessState {
  std::uint64_t start = 0;
  int status = -1;
};
ProcessState process_state(std::int32_t pid) {
  std::string path = "/proc/" + std::to_string(pid) + "/stat";
  std::ifstream in(path);
  if (!in) {
    struct stat st{};
    if (stat(path.c_str(), &st) < 0 && errno == ENOENT)
      return {0, 0};
    return {};
  }
  std::string line;
  std::getline(in, line);
  auto end = line.rfind(')');
  if (end == std::string::npos)
    return {};
  std::istringstream fields(line.substr(end + 2));
  char state;
  fields >> state;
  std::string discard;
  for (int i = 4; i < 22; ++i)
    fields >> discard;
  std::uint64_t start = 0;
  fields >> start;
  if (!fields || !start)
    return {};
  return {start, state == 'Z' || state == 'X' ? 0 : 1};
}
Process self() {
  auto p = process_state(getpid());
  if (p.status != 1)
    throw Error("cannot establish process identity from /proc");
  return {getpid(), p.start};
}
bool dead(Process p) {
  auto current = process_state(p.pid);
  return current.status == 0 ||
         (current.status == 1 && current.start != p.start);
}
class Guard {
  Shared *s_;

public:
  explicit Guard(Shared *s) : s_(s) {
    int e = pthread_mutex_lock(&s_->mutex);
    if (e == EOWNERDEAD) {
      // An interrupted metadata transaction cannot safely be guessed/repaired.
      s_->failed = true;
      pthread_mutex_consistent(&s_->mutex);
    } else if (e)
      throw Error("shared mutex lock: " + std::string(std::strerror(e)));
  }
  ~Guard() { pthread_mutex_unlock(&s_->mutex); }
  Guard(const Guard &) = delete;
};
void healthy(Shared *s) {
  if (s->failed)
    throw Error("domain metadata owner died; restart nodemaster and reconnect "
                "all nodes");
  if (s->stopped || dead(s->master))
    throw Error("nodemaster stopped; reconnect to a new domain session");
}
void reclaim(Slot &slot) {
  if (slot.state == 2 && !slot.pending && !slot.readers)
    slot.state = 0;
}
void remove_node(Shared *s, unsigned index) {
  auto mask = ~(std::uint32_t(1) << index);
  for (auto &t : s->topics) {
    t.subscribers &= mask;
    t.newest_subscribers &= mask;
  }
  for (auto &slot : s->slots) {
    if (slot.state == 1 && slot.writer == index)
      slot.state = 0;
    slot.pending &= mask;
    slot.readers &= mask;
    reclaim(slot);
  }
  s->nodes[index].used = false;
}
// A separate, persistent lock inode serializes creation and cleanup. Never
// unlink it: locking the data inode alone permits stale-cleanup/recreate races.
struct DomainLock {
  int fd = -1;
  explicit DomainLock(const std::string &name) {
    fd = shm_open((name + ".lock").c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0600);
    if (fd < 0)
      system_error("open domain lifecycle lock");
    struct stat st{};
    if (fstat(fd, &st) < 0 || st.st_uid != geteuid() || (st.st_mode & 077)) {
      close(fd);
      fd = -1;
      throw Error("invalid domain lock ownership or permissions");
    }
    if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
      close(fd);
      fd = -1;
      throw Error("nodemaster or domain cleanup already active");
    }
  }
  ~DomainLock() {
    if (fd >= 0)
      close(fd);
  }
  DomainLock(const DomainLock &) = delete;
};
struct Mapping {
  int fd = -1;
  Shared *s = nullptr;
  ~Mapping() {
    if (s)
      munmap(s, sizeof(Shared));
    if (fd >= 0)
      close(fd);
  }
  void map() {
    void *p = mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
    if (p == MAP_FAILED)
      system_error("mmap");
    s = static_cast<Shared *>(p);
  }
};
class SharedTransport : public Transport {
  Mapping map_;
  unsigned index_ = max_nodes;
  Process process_;
  void check() {
    if (getpid() != process_.pid)
      throw Error("Node/loan used after fork; create a new Node in the child");
    healthy(map_.s);
  }
  void validate_topic(unsigned t) {
    if (t >= max_topics || !map_.s->topics[t].used)
      throw Error("invalid topic handle");
  }
  Slot &owned(const Buffer &b, unsigned state) {
    if (b.slot >= pool_slots)
      throw Error("invalid loan slot");
    auto &s = map_.s->slots[b.slot];
    if (s.generation != b.generation || s.state != state || b.data != s.payload)
      throw Error("stale or invalid loan");
    return s;
  }

public:
  SharedTransport(const std::string &domain, const std::string &name)
      : process_(self()) {
    check_name(name, sizeof(Participant::name));
    map_.fd = shm_open(object_name(domain).c_str(), O_RDWR | O_CLOEXEC, 0600);
    if (map_.fd < 0)
      system_error("connect shared memory (start nodemaster first)");
    struct stat st{};
    if (fstat(map_.fd, &st) < 0)
      system_error("fstat");
    if (st.st_uid != geteuid() || (st.st_mode & 077) ||
        st.st_size != sizeof(Shared))
      throw Error("shared domain ownership, permissions, or ABI mismatch");
    // Master holds an exclusive flock. A shared try-lock succeeding means no
    // master.
    if (flock(map_.fd, LOCK_SH | LOCK_NB) == 0) {
      flock(map_.fd, LOCK_UN);
      throw Error("no live nodemaster; clean up stale domain");
    }
    if (errno != EWOULDBLOCK)
      system_error("check nodemaster lock");
    map_.map();
    // Initializer publishes signature last using an atomic release store.
    if (__atomic_load_n(&map_.s->signature, __ATOMIC_ACQUIRE) != magic ||
        map_.s->version != layout_version || map_.s->size != sizeof(Shared))
      throw Error("domain is not ready or has an incompatible ABI");
    Guard lock(map_.s);
    check();
    for (unsigned i = 0; i < max_nodes; ++i) {
      auto &n = map_.s->nodes[i];
      if (n.used && name == n.name)
        throw Error("node name already registered");
      if (!n.used && index_ == max_nodes)
        index_ = i;
    }
    if (index_ == max_nodes)
      throw Error("domain participant limit reached (32)");
    auto &n = map_.s->nodes[index_];
    n.process = process_;
    std::memcpy(n.name, name.c_str(), name.size() + 1);
    n.used = true;
  }
  ~SharedTransport() override {
    if (index_ == max_nodes || getpid() != process_.pid)
      return;
    try {
      Guard lock(map_.s);
      if (!map_.s->failed)
        remove_node(map_.s, index_);
    } catch (...) {
    }
  }
  unsigned topic(const std::string &name, Schema schema) override {
    check_name(name, sizeof(Topic::name), true);
    if (!schema.id || !schema.size || schema.size > max_payload ||
        !schema.alignment || schema.alignment > 64 ||
        (schema.alignment & (schema.alignment - 1)))
      throw Error("invalid message schema");
    Guard lock(map_.s);
    check();
    unsigned free = max_topics;
    for (unsigned i = 0; i < max_topics; ++i) {
      auto &t = map_.s->topics[i];
      if (!t.used) {
        if (free == max_topics)
          free = i;
        continue;
      }
      if (name == t.name) {
        if (t.schema != schema.id || t.size != schema.size ||
            t.alignment != schema.alignment)
          throw Error("topic schema/layout mismatch: " + name);
        return i;
      }
    }
    if (free == max_topics)
      throw Error("domain topic limit reached (32; restart to clear)");
    auto &t = map_.s->topics[free];
    t.schema = schema.id;
    t.size = schema.size;
    t.alignment = schema.alignment;
    t.subscribers = 0;
    t.newest_subscribers = 0;
    std::memcpy(t.name, name.c_str(), name.size() + 1);
    t.used = true;
    return free;
  }
  void subscribe(unsigned t) override {
    subscribe(t, SubscriptionPolicy::BLOCK_NEXT);
  }
  void subscribe(unsigned t, SubscriptionPolicy policy) override {
    if (policy != SubscriptionPolicy::BLOCK_NEXT &&
        policy != SubscriptionPolicy::POLL_NEWEST)
      throw Error("invalid subscription policy");
    Guard lock(map_.s);
    check();
    validate_topic(t);
    auto bit = std::uint32_t(1) << index_;
    if (map_.s->topics[t].subscribers & bit)
      throw Error("one subscriber per node/topic is supported");
    map_.s->topics[t].subscribers |= bit;
    if (policy == SubscriptionPolicy::POLL_NEWEST)
      map_.s->topics[t].newest_subscribers |= bit;
  }
  void unsubscribe(unsigned t) noexcept override {
    if (getpid() != process_.pid)
      return;
    try {
      Guard lock(map_.s);
      if (map_.s->failed || t >= max_topics)
        return;
      auto mask = ~(std::uint32_t(1) << index_);
      map_.s->topics[t].subscribers &= mask;
      map_.s->topics[t].newest_subscribers &= mask;
      for (auto &s : map_.s->slots)
        if (s.topic == t) {
          s.pending &= mask;
          reclaim(s);
        }
    } catch (...) {
    }
  }
  std::optional<Buffer> loan(unsigned t) override {
    Guard lock(map_.s);
    check();
    validate_topic(t);
    for (unsigned i = 0; i < pool_slots; ++i) {
      auto &s = map_.s->slots[i];
      if (s.state)
        continue;
      if (s.generation == std::numeric_limits<std::uint64_t>::max())
        throw Error("slot generation exhausted");
      ++s.generation;
      s.state = 1;
      s.writer = index_;
      s.topic = t;
      s.pending = s.readers = 0;
      // Clear padding and bounded storage as well as logical fields.
      std::memset(s.payload, 0, map_.s->topics[t].size);
      return Buffer{s.payload, i, s.generation, 0};
    }
    return std::nullopt;
  }
  void publish(unsigned t, const Buffer &b) override {
    Guard lock(map_.s);
    check();
    validate_topic(t);
    auto &s = owned(b, 1);
    if (s.writer != index_ || s.topic != t)
      throw Error("loan owner/topic mismatch");
    if (map_.s->sequence == std::numeric_limits<std::uint64_t>::max())
      throw Error("sequence exhausted");
    // Coalesce only pending deliveries. Active reader loans remain immutable,
    // and ordered subscribers keep every one of their pending messages.
    auto newest = map_.s->topics[t].newest_subscribers;
    if (newest)
      for (auto &previous : map_.s->slots)
        if (previous.state == 2 && previous.topic == t) {
          previous.pending &= ~newest;
          reclaim(previous);
        }
    s.sequence = ++map_.s->sequence;
    s.pending = map_.s->topics[t].subscribers;
    s.readers = 0;
    s.state = 2;
    reclaim(s);
  }
  std::optional<Buffer> take(unsigned t,
                             std::chrono::milliseconds timeout) override {
    if (timeout.count() < 0 || timeout > std::chrono::hours(24))
      throw Error("timeout must be between 0 and 24 hours");
    auto deadline = std::chrono::steady_clock::now() + timeout;
    do {
      {
        Guard lock(map_.s);
        check();
        validate_topic(t);
        auto bit = std::uint32_t(1) << index_;
        if (!(map_.s->topics[t].subscribers & bit))
          throw Error("not subscribed");
        unsigned best = pool_slots;
        for (unsigned i = 0; i < pool_slots; ++i) {
          auto &s = map_.s->slots[i];
          if (s.state == 2 && s.topic == t && (s.pending & bit) &&
              (best == pool_slots || s.sequence < map_.s->slots[best].sequence))
            best = i;
        }
        if (best != pool_slots) {
          auto &s = map_.s->slots[best];
          s.readers |= bit;
          s.pending &= ~bit;
          return Buffer{s.payload, best, s.generation, s.sequence};
        }
      }
      if (std::chrono::steady_clock::now() >= deadline)
        break;
      std::this_thread::sleep_until(
          std::min(deadline, std::chrono::steady_clock::now() +
                                 std::chrono::milliseconds(2)));
    } while (true);
    return std::nullopt;
  }
  void cancel(const Buffer &b) noexcept override {
    if (getpid() != process_.pid)
      return;
    try {
      Guard lock(map_.s);
      if (!map_.s->failed) {
        auto &s = owned(b, 1);
        if (s.writer == index_)
          s.state = 0;
      }
    } catch (...) {
    }
  }
  void release(const Buffer &b) noexcept override {
    if (getpid() != process_.pid)
      return;
    try {
      Guard lock(map_.s);
      if (!map_.s->failed) {
        auto &s = owned(b, 2);
        s.readers &= ~(std::uint32_t(1) << index_);
        reclaim(s);
      }
    } catch (...) {
    }
  }
};
} // namespace
struct LocalMaster::Impl {
  std::string name;
  DomainLock lifecycle;
  Mapping map;
  Process process = self();
  explicit Impl(const std::string &domain)
      : name(object_name(domain)), lifecycle(name) {
    map.fd =
        shm_open(name.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (map.fd < 0)
      system_error("create domain (if stale, use nodemaster --cleanup DOMAIN)");
    try {
      if (flock(map.fd, LOCK_EX | LOCK_NB) < 0)
        system_error("lock domain");
      if (ftruncate(map.fd, sizeof(Shared)) < 0)
        system_error("size domain");
      map.map();
      std::memset(map.s, 0, sizeof(Shared));
      pthread_mutexattr_t attr;
      int e = pthread_mutexattr_init(&attr);
      if (e)
        throw Error("mutex attributes: " + std::string(std::strerror(e)));
      e = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
      if (!e)
        e = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
      if (!e)
        e = pthread_mutex_init(&map.s->mutex, &attr);
      pthread_mutexattr_destroy(&attr);
      if (e)
        throw Error("initialize shared mutex: " +
                    std::string(std::strerror(e)));
      map.s->version = layout_version;
      map.s->size = sizeof(Shared);
      map.s->master = process;
      __atomic_store_n(&map.s->signature, magic, __ATOMIC_RELEASE);
    } catch (...) {
      shm_unlink(name.c_str());
      throw;
    }
  }
  ~Impl() {
    if (getpid() != process.pid)
      return;
    try {
      Guard lock(map.s);
      map.s->stopped = true;
    } catch (...) {
    }
    shm_unlink(name.c_str());
    // Never destroy a process-shared mutex while old mappings may still use it.
  }
};
LocalMaster::LocalMaster(const std::string &domain)
    : impl_(std::make_unique<Impl>(domain)) {}
LocalMaster::~LocalMaster() = default;
void LocalMaster::reap() {
  if (getpid() != impl_->process.pid)
    throw Error("master used after fork");
  auto *s = impl_->map.s;
  Guard lock(s);
  healthy(s);
  for (unsigned i = 0; i < max_nodes; ++i)
    if (s->nodes[i].used && dead(s->nodes[i].process))
      remove_node(s, i);
}
void LocalMaster::cleanup(const std::string &domain) {
  auto name = object_name(domain);
  DomainLock lifecycle(name);
  Mapping map;
  map.fd = shm_open(name.c_str(), O_RDWR | O_CLOEXEC, 0600);
  if (map.fd < 0) {
    if (errno == ENOENT)
      return;
    system_error("open stale domain");
  }
  struct stat st{};
  if (fstat(map.fd, &st) < 0)
    system_error("stat stale domain");
  if (st.st_uid != geteuid())
    throw Error("stale domain belongs to another user");
  if (flock(map.fd, LOCK_EX | LOCK_NB) < 0)
    throw Error("cannot clean up an active nodemaster");
  if (shm_unlink(name.c_str()) < 0 && errno != ENOENT)
    system_error("unlink stale domain");
}
#ifdef JK_RUNTIME_TESTING
[[noreturn]] void test_abandon_metadata(const std::string &domain) {
  Mapping map;
  map.fd = shm_open(object_name(domain).c_str(), O_RDWR | O_CLOEXEC, 0600);
  if (map.fd < 0)
    system_error("test open");
  map.map();
  Guard lock(map.s);
  _exit(77);
}
#endif
std::shared_ptr<Transport> shared_memory_transport(const std::string &domain,
                                                   const std::string &name) {
  return std::make_shared<SharedTransport>(domain, name);
}
} // namespace jk
