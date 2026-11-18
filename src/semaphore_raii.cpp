#include "semaphore_raii.h"

#include <cerrno>

#include "map_errno.h"

namespace semaphore_raii {

Semaphore_RAII::Semaphore_RAII(int id) : sem(new sem_t, [] (sem_t* s) {sem_destroy(s); delete s;}), id_(id) {
  if (sem_init(sem.get(), 0, 1) != 0) {
    throw map_errno::map_errno(errno);
  }
}

void Semaphore_RAII::wait() {
  if (sem_wait(sem.get()) != 0) {
    throw map_errno::map_errno(errno);
  }
}

Semaphore_RAII::WaitResult Semaphore_RAII::try_wait() {
  if (sem_trywait(sem.get()) != 0) {
    return NOT_AQUIRED;
  }
  return AQUIRED;
}

void Semaphore_RAII::post() {
  int rv;
  if (sem_getvalue(sem.get(), &rv) != 0) {
    throw map_errno::map_errno(errno);
  }
  if (rv >= 1) {
    return;
  }
  if (sem_post(sem.get()) != 0) {
    throw map_errno::map_errno(errno);
  }
}

int Semaphore_RAII::id() {
  return id_;
}

}  // namespace semaphore_raii
