#pragma once
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <jk/message.hpp>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace jk {
inline constexpr std::size_t max_payload = 65536;
inline constexpr std::size_t pool_slots = 32;
class Error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};
struct Buffer {
  void *data = nullptr;
  std::uint32_t slot = 0;
  std::uint64_t generation = 0, sequence = 0;
};
struct Schema {
  std::uint64_t id;
  std::size_t size, alignment;
};
// Local transport boundary. Future backends can implement the same loan
// contract. An endpoint is process-bound: create new Nodes after fork; never
// inherit loans.
class Transport {
public:
  virtual ~Transport() = default;
  virtual unsigned topic(const std::string &, Schema) = 0;
  virtual void subscribe(unsigned) = 0;
  virtual void unsubscribe(unsigned) noexcept = 0;
  virtual std::optional<Buffer> loan(unsigned) = 0;
  virtual void publish(unsigned, const Buffer &) = 0;
  virtual std::optional<Buffer> take(unsigned, std::chrono::milliseconds) = 0;
  virtual void cancel(const Buffer &) noexcept = 0;
  virtual void release(const Buffer &) noexcept = 0;
};
std::shared_ptr<Transport>
shared_memory_transport(const std::string &domain,
                        const std::string &node_name);
class LocalMaster {
public:
  explicit LocalMaster(const std::string &domain);
  ~LocalMaster();
  LocalMaster(const LocalMaster &) = delete;
  LocalMaster &operator=(const LocalMaster &) = delete;
  void reap();
  static void cleanup(const std::string &domain);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

template <class T> class Publisher;
template <class T> class Subscriber;
template <class T> class WriteLoan {
  friend class Publisher<T>;
  std::shared_ptr<Transport> transport_;
  unsigned topic_ = 0;
  Buffer buffer_;
  WriteLoan(std::shared_ptr<Transport> t, unsigned topic, Buffer b)
      : transport_(std::move(t)), topic_(topic), buffer_(b) {
    // Generated messages are trivial, pointer-free, native-layout values.
    ::new (buffer_.data) T; // Transport already cleared the allocation once.
  }

public:
  ~WriteLoan() {
    if (transport_)
      transport_->cancel(buffer_);
  }
  WriteLoan(const WriteLoan &) = delete;
  WriteLoan &operator=(const WriteLoan &) = delete;
  WriteLoan(WriteLoan &&o) noexcept
      : transport_(std::move(o.transport_)), topic_(o.topic_),
        buffer_(o.buffer_) {}
  WriteLoan &operator=(WriteLoan &&) = delete;
  T *operator->() {
    if (!transport_)
      throw Error("loan already consumed");
    return static_cast<T *>(buffer_.data);
  }
  T &operator*() { return *operator->(); }
  std::uint32_t slot() const { return buffer_.slot; }
  std::uint64_t generation() const { return buffer_.generation; }
};
template <class T> class ReadLoan {
  friend class Subscriber<T>;
  std::shared_ptr<Transport> transport_;
  Buffer buffer_;
  ReadLoan(std::shared_ptr<Transport> t, Buffer b)
      : transport_(std::move(t)), buffer_(b) {}

public:
  ~ReadLoan() {
    if (transport_)
      transport_->release(buffer_);
  }
  ReadLoan(const ReadLoan &) = delete;
  ReadLoan &operator=(const ReadLoan &) = delete;
  ReadLoan(ReadLoan &&o) noexcept
      : transport_(std::move(o.transport_)), buffer_(o.buffer_) {}
  ReadLoan &operator=(ReadLoan &&) = delete;
  const T *operator->() const {
    if (!transport_)
      throw Error("loan moved");
    return static_cast<const T *>(buffer_.data);
  }
  const T &operator*() const { return *operator->(); }
  std::uint32_t slot() const { return buffer_.slot; }
  std::uint64_t generation() const { return buffer_.generation; }
  std::uint64_t sequence() const { return buffer_.sequence; }
};
template <class T> Schema schema() {
  static_assert(std::is_trivial_v<T> && std::is_standard_layout_v<T>);
  static_assert(sizeof(T) <= max_payload && alignof(T) <= 64);
  return {MessageTraits<T>::schema, sizeof(T), alignof(T)};
}
template <class T> class Publisher {
  std::shared_ptr<Transport> transport_;
  unsigned topic_;

public:
  Publisher(std::shared_ptr<Transport> t, const std::string &name)
      : transport_(std::move(t)), topic_(transport_->topic(name, schema<T>())) {
  }
  std::optional<WriteLoan<T>> loan() {
    auto b = transport_->loan(topic_);
    if (!b)
      return std::nullopt;
    return WriteLoan<T>(transport_, topic_, *b);
  }
  void publish(WriteLoan<T> &&loan) {
    if (loan.transport_ != transport_ || loan.topic_ != topic_)
      throw Error("loan belongs to another publisher");
    if (!MessageTraits<T>::valid(*loan))
      throw Error("message violates its schema bounds");
    transport_->publish(topic_, loan.buffer_);
    loan.transport_.reset();
  }
};
template <class T> class Subscriber {
  std::shared_ptr<Transport> transport_;
  unsigned topic_;

public:
  Subscriber(std::shared_ptr<Transport> t, const std::string &name)
      : transport_(std::move(t)), topic_(transport_->topic(name, schema<T>())) {
    transport_->subscribe(topic_);
  }
  ~Subscriber() {
    if (transport_)
      transport_->unsubscribe(topic_);
  }
  Subscriber(const Subscriber &) = delete;
  Subscriber &operator=(const Subscriber &) = delete;
  Subscriber(Subscriber &&o) noexcept
      : transport_(std::move(o.transport_)), topic_(o.topic_) {}
  Subscriber &operator=(Subscriber &&) = delete;
  std::optional<ReadLoan<T>>
  take(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
    auto b = transport_->take(topic_, timeout);
    if (!b)
      return std::nullopt;
    ReadLoan<T> loan(transport_, *b);
    if (!MessageTraits<T>::valid(*loan))
      throw Error("received invalid message");
    return loan;
  }
};
class Node {
  std::shared_ptr<Transport> transport_;

public:
  Node(const std::string &domain, const std::string &name)
      : transport_(shared_memory_transport(domain, name)) {}
  explicit Node(std::shared_ptr<Transport> t) : transport_(std::move(t)) {
    if (!transport_)
      throw Error("null transport");
  }
  template <class T> Publisher<T> publisher(const std::string &topic) {
    return {transport_, topic};
  }
  template <class T> Subscriber<T> subscriber(const std::string &topic) {
    return {transport_, topic};
  }
};
} // namespace jk
