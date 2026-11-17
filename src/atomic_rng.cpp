#include "atomic_rng.h"

int AtomicRng::unlocked_rand(int min, int max) {
  return std::uniform_int_distribution<>(min,max)(shared_state->gen);
}

int AtomicRng::rand_in_range(int min, int max) {
    std::lock_guard<std::mutex> lock(shared_state->mutex);
    return unlocked_rand(min, max);
  }
