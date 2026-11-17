#include "thread_manager.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <syncstream>

#include "atomic_rng.h"
#include "pthread_raii.h"
#include "semaphore_raii.h"

namespace thread_manager {

constexpr int INITIAL_WORK = 10;

ThreadManager::ThreadManager(int n_threads, int n_semaphores)
    : n_threads(n_threads), n_semaphores(n_semaphores) {
  semaphores.reserve(n_semaphores);
  std::ranges::copy(
      std::views::iota(0, n_semaphores) | std::views::transform([](int i) {
        return semaphore_raii::Semaphore_RAII{i};
      }),
      semaphores.begin());
}

int ThreadManager::run() {
  char initial_id = 'A';
  for (int i = 0; i < n_threads; i += 1) {
    threads.push_back(pthread_raii::PThread_RAII{
        std::make_unique<thread_manager::ThreadManager::Thread>(
            initial_id + i, INITIAL_WORK, semaphores, rng)});
  }
  for (auto& t : threads) {
    t.join();
  }
  return 0;
}

ThreadManager::Thread::Thread(char id, int initial_work,
                              std::vector<semaphore_raii::Semaphore_RAII> sems,
                              atomic_rng::AtomicRng rng)
    : id(id), sems(sems), rng(rng), work(initial_work) {}

static void sleep(char id, atomic_rng::AtomicRng rng) {
#ifdef THREAD_MANAGER_DO_SLEEP
  constexpr int MILLIS = 1000000;
  const timespec duration = {.tv_sec = 0,
                             .tv_nsec = rng.rand_in_range(0, 10) * MILLIS};
  std::osyncstream(std::cout)
      << id << " sleeping for " << duration.tv_nsec << "\n";
  nanosleep(&duration, nullptr);
#else
  return;  // if not defined just no-op the function
#endif
}

static void post_all(char id,
                     std::vector<semaphore_raii::Semaphore_RAII>& sems) {
  for (auto& s : sems) {
    s.post();
    std::osyncstream(std::cout) << id << " released " << s.id() << "\n";
  }
}

void* ThreadManager::Thread::do_work() {
  std::vector<semaphore_raii::Semaphore_RAII> held;
  int n_sems = sems.size();
  while (work > 0) {
    bool all_held = false;
    int n_sems_needed = rng.rand_in_range(1, n_sems);
    std::osyncstream(std::cout) << id << " needs " << n_sems_needed << "\n";
    std::vector<semaphore_raii::Semaphore_RAII> sems_needed =
        rng.shuffle_n(sems, n_sems_needed);
    while (!all_held) {
      all_held = true;
      for (auto& s : sems_needed) {
        if (s.try_wait() ==
            semaphore_raii::Semaphore_RAII::WaitResult::NOT_AQUIRED) {
          post_all(id, held);
          held.clear();
          all_held = false;
          sleep(id, rng);
          break;
        }
        held.push_back(s);
        std::osyncstream(std::cout) << id << " got " << s.id() << "\n";
      }
    }
    work -= 1;
    std::osyncstream(std::cout) << id << " has " << work << " left\n";
    post_all(id, held);
    held.clear();
    sleep(id, rng);
  }
  pthread_exit(nullptr);
}

}  // namespace thread_manager
