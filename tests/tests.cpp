#include <mpsc/mpsc_channel.hpp>

#include <catch.hpp>

#include <thread>

TEST_CASE("Basic usage") {
  auto [sender, receiver] = mpsc::Channel<int>::make();

  sender.send(1);
  auto result = receiver.receive();
  REQUIRE(result == 1);
}

TEST_CASE("Multiple senders") {
  auto [sender1, receiver] = mpsc::Channel<int>::make();
  auto sender2 = sender1;

  sender1.send(1);
  sender2.send(2);

  std::set<int> results;
  for (int i = 0; i < 2; ++i) {
    auto result = receiver.receive();
    results.insert(*result);
  }
  REQUIRE(results == std::set<int>{1, 2});
}

TEST_CASE("From several threads") {
  auto channel = mpsc::Channel<int>::make();
  auto &sender = channel.first;
  auto receiver = std::move(channel.second);

  constexpr int k_size = 1000;

  auto producer = std::jthread([sender]() mutable {
    {
      std::vector<std::jthread> threads;
      for (int i = 0; i < k_size; ++i) {
        threads.emplace_back([sender, i]() mutable { sender.send(i); });
      }
    }
    sender.close();
  });

  std::vector<int> results(receiver.begin(), mpsc::Receiver<int>::end());
  std::sort(std::begin(results), std::end(results));

  std::vector<int> expected(k_size);
  std::iota(std::begin(expected), std::end(expected), 0);

  REQUIRE(results == expected);
}

TEST_CASE("move only type") {
  auto [sender, receiver] = mpsc::Channel<std::unique_ptr<int>>::make();

  sender.send(std::make_unique<int>(1));
  auto result = receiver.receive();
  REQUIRE(**result == 1);
}
