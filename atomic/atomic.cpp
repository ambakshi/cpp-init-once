#include <future>
#include <iostream>
#include <atomic>

std::atomic<int> value;

int loop(bool inc, int limit) {
  std::cout << "Started " << inc << " " << limit << std::endl;
  for (int i = 0; i < limit; ++i) {
    if (inc) {
      std::atomic_fetch_add(&value, 1);
    } else {
      std::atomic_fetch_sub(&value, 1);
    }
  }
  return 0;
}

int main() {
  value = 0;
  auto f = std::async(std::launch::async, std::bind(loop, true, 20000000));
  loop(false, 10000000);
  f.wait();
  std::cout << std::atomic_load(&value) << std::endl;
}
