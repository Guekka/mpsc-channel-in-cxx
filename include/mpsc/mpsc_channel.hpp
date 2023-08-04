// File from
// https://github.com/liuchibing/mpsc-channel-in-cxx/blob/master/mpsc_channel.hpp
// But largely modified. In my opinion, this is enough to justify a license
// change.

/*
mpsc_channel.hpp
A simple C++ implementation of the 'channel' in rust language (std::mpsc).

# Usage
Use `mpsc::make_channel<T>` to create a channel. `make_channel<T>` will return a
tuple of (Sender<T>, Receiver<T>).

For example:
```c++
// Create.
auto [ sender, receiver ] = mpsc::make_channel<int>();

// Send.
sender.send(3);

// Receive (both returns a std::optional<T>.)
receiver.receive(); // Blocking when there is nothing present in the channel.
receiver.try_receive(); // Not blocking. Return immediately.

// close() and closed()
sender.close();
bool result = sender.closed();
assert(result == receiver.closed());

// You can use range-based for loop to receive from the channel.
for (int v: receiver) {
        // do something with v
        // The loop will stop after the sender called close() and all items were
processed.
        // Only sender can call close().
}
```

Note: `mpsc` stands for Multi-Producer Single-Consumer. So `Sender` can be
either copied and moved, but `Receiver` can only be moved.

Feel free to explore the `tests.cpp`. The tests are also examples of the usage.

Read the source if you need more information. Sorry for the lack of comments.
～(￣▽￣～)~

Copyright (c) 2019 liuchibing.
*/

#pragma once

#include <condition_variable>
#include <exception>
#include <iterator>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mpsc {

template <typename T> class Sender;
template <typename T> class Receiver;

class channel_closed_exception : std::logic_error {
public:
  channel_closed_exception()
      : std::logic_error("This channel has been closed.") {}
};

template <typename T> class Channel {
private:
  Channel() = default;

  std::queue<T, std::list<T>> queue;
  std::mutex mutex;
  std::condition_variable condvar;
  bool need_notify = false;
  bool _closed = false;

public:
  Channel(const Channel<T> &) = delete;
  Channel(Channel<T> &&) = delete;

  Channel<T> &operator=(const Channel<T> &) = delete;
  Channel<T> &operator=(Channel<T> &&) = delete;

  [[nodiscard]] static auto make() {
    static_assert(std::is_copy_constructible_v<T> ||
                      std::is_move_constructible_v<T>,
                  "T should be copy-constructible or move-constructible.");

    // not using std::make_shared because it requires public constructor.
    auto channel = std::shared_ptr<Channel<T>>(new Channel<T>());
    Sender<T> sender{channel};
    Receiver<T> receiver{channel};
    return std::pair<Sender<T>, Receiver<T>>{std::move(sender),
                                             std::move(receiver)};
  }

  template <typename U>
    requires std::is_same_v<std::decay_t<U>, T>
  void send(U &&value) {
    std::unique_lock lock(mutex);
    if (_closed) {
      throw channel_closed_exception();
    }
    queue.push(std::move(value));
    if (need_notify) {
      need_notify = false;
      lock.unlock();
      condvar.notify_one();
    }
  }

  std::optional<T> receive() {
    std::unique_lock lock(mutex);
    if (queue.empty()) {
      need_notify = true;
      condvar.wait(lock, [this] { return !queue.empty() || _closed; });
    }
    if (queue.empty()) {
      return std::nullopt;
    }
    T result = std::move(queue.front());
    queue.pop();
    return result;
  }

  void close() {
    std::unique_lock lock(mutex);
    _closed = true;
    if (need_notify) {
      need_notify = false;
      lock.unlock();
      condvar.notify_one();
    }
  }
  bool closed() { return _closed; }
};

template <typename T> class Sender {
public:
  Sender(const Sender<T> &s) : channel(s.channel), thread_id(s.thread_id + 1) {}
  Sender(Sender<T> &&) = default;

  Sender<T> &operator=(const Sender<T> &) = delete;
  Sender<T> &operator=(Sender<T> &&) = default;

  template <typename U>
    requires std::is_same_v<std::decay_t<U>, T>
  void send(U &&value) {
    validate();
    channel->send(std::forward<U>(value));
  }

  void close() {
    validate();
    channel->close();
  }

  bool closed() {
    validate();
    return channel->closed();
  }

  explicit operator bool() const { return static_cast<bool>(channel); }

private:
  explicit Sender(std::shared_ptr<Channel<T>> channel) : channel(channel){};

  std::shared_ptr<Channel<T>> channel;
  // Our wait queue requires a thread id to be passed in. For simplicity, we
  // just use a copy counter
  std::uint16_t thread_id = 0;

  void validate() {
    if (!channel) {
      throw std::invalid_argument("This sender has been moved out.");
    }
  }

  friend class Channel<T>;
};

template <typename T> class Receiver {
public:
  Receiver(Receiver<T> &&) = default;
  Receiver<T> &operator=(Receiver<T> &&) = default;

  Receiver(const Receiver<T> &) = delete;
  Receiver<T> &operator=(const Receiver<T> &) = delete;

  std::optional<T> receive() {
    validate();
    return channel->receive();
  }

  bool closed() {
    validate();
    return channel->closed();
  }

  explicit operator bool() const { return static_cast<bool>(channel); }

private:
  explicit Receiver(std::shared_ptr<Channel<T>> channel) : channel(channel){};

  std::shared_ptr<Channel<T>> channel;

  void validate() {
    if (!channel) {
      throw std::invalid_argument("This receiver has been moved out.");
    }
  }

  friend class Channel<T>;

public:
  class iterator {
  private:
  public:
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;
    using pointer = T *;
    using reference = T &;
    using value_type = T;

    iterator() : receiver(nullptr) {}
    explicit iterator(Receiver<T> &receiver) : receiver(&receiver) { next(); }

    reference operator*() { return current.value(); }
    pointer operator->() { return &current.value(); }
    iterator &operator++() {
      next();
      return *this;
    }
    iterator operator++(int) = delete;
    bool operator==(const iterator &other) const {
      return receiver == nullptr && other.receiver == nullptr;
    }
    bool operator!=(const iterator &other) const { return !(*this == other); }

  private:
    Receiver<T> *receiver;
    std::optional<T> current = std::nullopt;
    void next();
  };

  iterator begin() { return iterator(*this); }

  static iterator end() { return iterator(); }
};

/* ======== Implementations ========= */

template <typename T> void Receiver<T>::iterator::next() {
  if (!receiver)
    return;

  std::optional<T> tmp = receiver->receive();
  if (!tmp.has_value()) {
    receiver = nullptr;
    current.reset();
    return;
  }

  current.emplace(std::move(tmp.value()));
}
} // namespace mpsc
