#include "thread_manager.h"

#include <algorithm>
#include <iostream>
#include <ranges>
#include <syncstream>

#include "atomic_rng.h"
#include "pthread_raii.h"
#include "semaphore_raii.h"

#define THREAD_MANAGER_DO_SLEEP
#define THREAD_MANAGER_TERSE_LOGS
namespace thread_manager {

constexpr int INITIAL_WORK = 10;

ThreadManager::ThreadManager(int n_threads, int n_semaphores)
    : n_threads(n_threads), n_semaphores(n_semaphores) {
  semaphores.reserve(n_semaphores);
  std::ranges::copy(
      std::views::iota(0, n_semaphores) | std::views::transform([](int i) {
        return semaphore_raii::Semaphore_RAII{i};
      }),
      std::back_inserter(semaphores));
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

void ThreadManager::Thread::post_all(
    std::vector<semaphore_raii::Semaphore_RAII>& sems) {
  for (auto& s : sems) {
    post_sem(s);
  }
}

void ThreadManager::Thread::sleep_rand() {
#ifdef THREAD_MANAGER_DO_SLEEP
  constexpr int MILLIS = 1000000;
  const timespec duration = {.tv_sec = 0,
                             .tv_nsec = rng.rand_in_range(0, 10) * MILLIS};
#ifndef THREAD_MANAGER_TERSE_LOGS
  std::osyncstream(std::cout)
      << id << " sleeping for " << duration.tv_nsec << "\n";
#endif
  nanosleep(&duration, nullptr);
#else
  return;  // if not defined just no-op the function
#endif
}

bool ThreadManager::Thread::take_sem(semaphore_raii::Semaphore_RAII s) {
  if (s.try_wait() == semaphore_raii::Semaphore_RAII::WaitResult::NOT_AQUIRED) {
    return false;
  }
  std::osyncstream(std::cout) << id <<
#ifdef THREAD_MANAGER_TERSE_LOGS
    ">"
#else
    " got "
#endif
  << s.id() << "\n";
  return true;
}

void ThreadManager::Thread::post_sem(semaphore_raii::Semaphore_RAII s) {
  s.post();
  std::osyncstream(std::cout) << id <<
#ifdef THREAD_MANAGER_TERSE_LOGS
    "<"
#else
    " released "
#endif
  << s.id() << "\n";
}

ThreadManager::Thread::Thread(char id, int initial_work,
                              std::vector<semaphore_raii::Semaphore_RAII> sems,
                              atomic_rng::AtomicRng rng)
    : id(id), sems(sems), rng(rng), work(initial_work) {}

void* ThreadManager::Thread::do_work() {
  std::vector<semaphore_raii::Semaphore_RAII> held;
  int n_sems = sems.size();
  while (work > 0) {
    bool all_held = false;
    int n_sems_needed = rng.rand_in_range(1, n_sems);
#ifndef THREAD_MANAGER_TERSE_LOGS
    std::osyncstream(std::cout) << id << " needs " << n_sems_needed << "\n";
#endif
    std::vector<semaphore_raii::Semaphore_RAII> sems_needed =
        rng.shuffle_n(sems, n_sems_needed);
    while (!all_held) {
      all_held = true;
      for (auto& s : sems_needed) {
        if (take_sem(s)) {
          held.push_back(s);
        } else {
          post_all(held);
          held.clear();
          all_held = false;
          sleep_rand();
        }
      }
    }
    work -= 1;
    std::osyncstream(std::cout) << id << " has " << work << " left\n";
    post_all(held);
    held.clear();
    sleep_rand();
  }
  pthread_exit(nullptr);
}

}  // namespace thread_manager
