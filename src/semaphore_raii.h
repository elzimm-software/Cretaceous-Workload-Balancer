#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <semaphore.h>

#include <memory>

namespace semaphore_raii {

/// RAII wrapper for sem_t
class Semaphore_RAII {
 private:
  std::shared_ptr<sem_t> sem;
  int id_;

 public:
  enum WaitResult { AQUIRED, NOT_AQUIRED };

  Semaphore_RAII(int id);

  void wait();
  WaitResult try_wait();

  void post();
  int id();
};

}  // namespace semaphore_raii

#endif  // SEMAPHORE_H_
