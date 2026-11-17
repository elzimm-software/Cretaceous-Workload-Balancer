#ifndef ATOMIC_RNG_H_
#define ATOMIC_RNG_H_

#include <mutex>
#include <vector>
#include <random>
#include <memory>

class AtomicRng {
 private:
  struct shared_state_t {
    std::mutex mutex;
    std::mt19937 gen{std::random_device{}()};
  };

  std::shared_ptr<shared_state_t> shared_state = std::make_shared<shared_state_t>();

  int unlocked_rand(int min, int max);
  std::vector<size_t> shuffle_indices(size_t len, size_t n);

 public:
  int rand_in_range(int min, int max);

  template <typename T>
  std::vector<T> shuffle_n(std::vector<T>& a, size_t n) {
    std::vector<size_t> idxs = shuffle_indices(a.size(), n);
    for (size_t i = 0; i < n; i += 1) {
      std::swap(a[i], a[idxs[i]]);
    }
    return {a.begin(), a.begin() + n};
  }
};


#endif // ATOMIC_RNG_H_
