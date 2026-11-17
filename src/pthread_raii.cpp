#include "pthread_raii.h"
#include <cerrno>
#include "map_errno.h"

namespace pthread_raii {

void* PThread_RAII::thread_runner(void* ptr) {
  std::unique_ptr<Worker> worker(static_cast<Worker*>(ptr));
  return worker->do_work();
}

PThread_RAII::PThread_RAII(std::unique_ptr<PThread_RAII::Worker> worker) {
  if (pthread_create(&thread, nullptr, thread_runner, worker.release()) != 0) {
    throw map_errno::map_errno(errno);
  }
  running = true;
}

PThread_RAII::~PThread_RAII() {
  join();
}

void PThread_RAII::join() {
  if (!running) {
    return;
  }
  if (pthread_join(thread, nullptr) != 0) {
    throw map_errno::map_errno(errno);
  }
  running = false;
}

}  // namespace pthread_raii
