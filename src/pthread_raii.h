#ifndef PTHREAD_RAII_H_
#define PTHREAD_RAII_H_

#include <pthread.h>

#include <memory>
#include <utility>

namespace pthread_raii {

class PThread_RAII {
 private:
  pthread_t thread;
  bool running = false;

  static void* thread_runner(void* ptr);

 public:
  class Worker {
   public:
    virtual ~Worker() = default;

    virtual void* do_work() = 0;
  };

  explicit PThread_RAII(std::unique_ptr<Worker> worker);

  // remove copy semantics
  PThread_RAII(const PThread_RAII&) = delete;
  PThread_RAII& operator=(const PThread_RAII&) = delete;
  PThread_RAII(PThread_RAII&& other) noexcept;
  PThread_RAII& operator=(PThread_RAII&& other) noexcept;

  ~PThread_RAII();

  void join();
};

}  // namespace pthread_raii

#endif  // PTHREAD_RAII_H_