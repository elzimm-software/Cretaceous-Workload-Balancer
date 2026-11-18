#include "pthread_raii.h"

#include <cerrno>
#include <utility>

#include "map_errno.h"

namespace pthread_raii {

void* PThread_RAII::thread_runner(void* ptr) {
  const std::unique_ptr<Worker> worker(static_cast<Worker*>(ptr));
  return worker->do_work();
}

PThread_RAII::PThread_RAII(std::unique_ptr<PThread_RAII::Worker> worker) {
  const int rc =
      pthread_create(&thread, nullptr, thread_runner, worker.release());
  if (rc != 0) {
    throw map_errno::map_errno(rc);
  }
  running = true;
}

PThread_RAII::~PThread_RAII() { join(); }

void PThread_RAII::join() {
  if (!running) {
    return;
  }
  const int rc = pthread_join(thread, nullptr);
  if (rc != 0) {
    throw map_errno::map_errno(rc);
  }
  running = false;
}

PThread_RAII::PThread_RAII(PThread_RAII&& other) noexcept
    : thread(other.thread), running(std::exchange(other.running, false)) {}
PThread_RAII& PThread_RAII::operator=(PThread_RAII&& other) noexcept {
  if (this != &other) {
    join();
    thread = other.thread;
    running = std::exchange(other.running, false);
  }
  return *this;
}

}  // namespace pthread_raii
