#include "atomic_rng.h"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <ranges>

namespace atomic_rng {

int AtomicRng::unlocked_rand(const int min, const int max) const {
  return std::uniform_int_distribution<>(min, max)(shared_state->gen);
}

int AtomicRng::rand_in_range(const int min, const int max) const {
  std::lock_guard<std::mutex> lock(shared_state->mutex);
  return unlocked_rand(min, max);
}

std::vector<size_t> AtomicRng::shuffle_indices(size_t len, size_t n) const {
  std::lock_guard<std::mutex> lock(shared_state->mutex);
  std::vector<size_t> v(n);
  std::ranges::copy(std::views::iota(static_cast<size_t>(0), n) |
                        std::views::transform([this, n](const size_t i) {
                          return unlocked_rand(static_cast<int>(i), static_cast<int>(n - 1));
                        }),
                    v.begin());
  return v;
}

}  // namespace atomic_rng
