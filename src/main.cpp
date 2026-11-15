
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

constexpr size_t N_THREADS = 4;

struct threads_t {
private:
    struct thread_args_t {
        sem_t* sem;
        char id;
    };

    static void *thread_do_work(void* ptr) {
        auto* args = static_cast<thread_args_t *>(ptr);
        sem_t* sem = args->sem;
        char id = args->id;
        delete args;
        int n = 10;
        while (n > 0) {
            std::cout << "thread " << id << " waiting" << std::endl;
            sem_wait(sem);
            n -= 1;
            std::cout << "thread " << id << " has " << n << " work remaining" << std::endl;
            sem_post(sem);
        }
        std::cout << "thread " << id << " work complete" << std::endl;
        pthread_exit(nullptr);
    }
public:
    pthread_t t[N_THREADS]{};
    int ret[N_THREADS]{};
    sem_t sem{};

    void run() {
        sem_init(&sem, 0,1);
        char next_id = 'A';
        for (int i = 0; i < N_THREADS; i += 1) {
            ret[i] = pthread_create(&t[i], nullptr, thread_do_work, new thread_args_t {.sem = &sem, .id = next_id});
            next_id += 1;
        }
        for (int i = 0; i < N_THREADS; i += 1) {
            pthread_join(t[i], nullptr);
        }
    }
};

int main(int argc, char** argv) {
    threads_t t;
    t.run();

    return 0;
}
