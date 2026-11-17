#include "thread_manager.h"

int main() {
  constexpr int N_THREADS = 5;
  constexpr int N_SEMAPHORES = 3;
  return thread_manager::ThreadManager{N_THREADS, N_SEMAPHORES}.run();
}
