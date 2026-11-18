#ifndef THREAD_MANAGER_H_
#define THREAD_MANAGER_H_

#include <vector>

#include "atomic_rng.h"
#include "pthread_raii.h"
#include "semaphore_raii.h"

namespace thread_manager {

class ThreadManager {
 private:
  class Thread : public pthread_raii::PThread_RAII::Worker {
    char id;
    std::vector<semaphore_raii::Semaphore_RAII> sems;
    atomic_rng::AtomicRng rng;
    int work;

    void post_all(
        const std::vector<semaphore_raii::Semaphore_RAII>& held) const;
    void sleep_rand() const;
    [[nodiscard]] bool take_sem(semaphore_raii::Semaphore_RAII s) const;
    void post_sem(semaphore_raii::Semaphore_RAII s) const;

   public:
    Thread(char id, int initial_work,
           std::vector<semaphore_raii::Semaphore_RAII> sems,
           atomic_rng::AtomicRng rng);

    void* do_work() override;
  };

  int n_threads;
  int n_semaphores;
  std::vector<pthread_raii::PThread_RAII> threads;
  std::vector<semaphore_raii::Semaphore_RAII> semaphores;
  atomic_rng::AtomicRng rng;

 public:
  ThreadManager(int n_threads, int n_semaphores);
  int run();
};

}  // namespace thread_manager

#endif  // THREAD_MANAGER_H_
