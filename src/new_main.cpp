#include <pthread.h>
#include <semaphore.h>

#include <algorithm>
#include <cerrno>
#include <iterator>
#include <memory>
#include <mutex>
#include <random>
#include <system_error>
#include <type_traits>
#include <vector>

#include "error.h"
#include "semaphore.h"
#include "atomic_rng.h"


class ThreadManager {
 private:
  struct thread_args_t {
    char id;
    std::vector<Semaphore> sems;
    AtomicRng rng;
  };

  static void* thread_do_work(void* ptr) {
    auto* args = static_cast<thread_args_t*>(ptr);
    return nullptr;
  }

  thread_args_t assemble_args(char id) {
    return {
        .id = id,
        .sems = semaphores,
        .rng = rng,
    };
  }

  std::vector<pthread_t> threads;
  std::vector<int> return_vals;
  std::vector<Semaphore> semaphores;
  AtomicRng rng;

 public:
  ThreadManager(int n_threads, int n_semaphores)
      : threads(std::vector<pthread_t>(n_threads)),
        return_vals(std::vector<int>(n_threads)),
        semaphores(std::vector<Semaphore>(n_semaphores)) {}

  int run() {
    char next_id = 'A';
    for (int i = 0; i < threads.capacity(); i += 1) {
      return_vals[0];
    }
    return 0;
  }
};

int main() {
  constexpr int N_THREADS = 5;
  constexpr int N_SEMAPHORES = 3;
  return ThreadManager{N_THREADS, N_SEMAPHORES}.run();
}
