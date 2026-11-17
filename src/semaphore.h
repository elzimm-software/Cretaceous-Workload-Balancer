#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <semaphore.h>
#include <cerrno>
#include <memory>

#include "error.h"


/// RAII wrapper for sem_t
class Semaphore {
 private:
  std::shared_ptr<sem_t> sem;

 public:
  enum WaitResult { AQUIRED, NOT_AQUIRED };

  Semaphore() : sem(std::make_shared<sem_t>()) {
    if (sem_init(sem.get(), 0, 1) != 0) {
      throw map_errno(errno);
    }
  }

  ~Semaphore() { sem_destroy(sem.get()); }

  void wait() {
    if (sem_wait(sem.get()) != 0) {
      throw map_errno(errno);
    }
  }

  WaitResult try_wait() {
    if (sem_trywait(sem.get()) != 0) {
      return NOT_AQUIRED;
    }
    return AQUIRED;
  }

  void post() {
    int rv;
    if (sem_getvalue(sem.get(), &rv) != 0) {
      throw map_errno(errno);
    }
    if (rv >= 1) {
      return;
    }
    if (sem_post(sem.get()) != 0) {
      throw map_errno(errno);
    }
  }
};


#endif // SEMAPHORE_H_
