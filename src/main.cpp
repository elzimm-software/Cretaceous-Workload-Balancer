
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <memory>
#include <random>

constexpr size_t N_THREADS = 5;
constexpr size_t N_SEMAPHORES = 3;

class Threads {
private:
    struct thread_args_t {
        sem_t** sem;
        char id;
        std::mt19937* gen;
    };

    static void *thread_do_work(void* ptr) {
        auto* args = static_cast<thread_args_t *>(ptr);
        sem_t **sem = args->sem;
        char id = args->id;
        std::mt19937* gen = args->gen;
        delete args;
        std::uniform_int_distribution<> sem_dist(1,N_SEMAPHORES);
        pthread_exit(nullptr);
    }
public:
    pthread_t t[N_THREADS]{};
    int ret[N_THREADS]{};
    sem_t* sem[N_SEMAPHORES]{};

    std::mt19937 gen;

    Threads() {
        std::random_device rd;
        gen = std::mt19937(rd());
    }

    void run() {
        for (int i = 0; i < N_SEMAPHORES; i += 1) {
            sem_init(sem[i], 0, 1);
        }
        char next_id = 'A';
        for (int i = 0; i < N_THREADS; i += 1) {
            ret[i] = pthread_create(&t[i], nullptr, thread_do_work, new thread_args_t {.sem = sem, .id = next_id, .gen = &gen});
            next_id += 1;
        }
        for (int i = 0; i < N_THREADS; i += 1) {
            pthread_join(t[i], nullptr);
        }
    }
};

int main(int argc, char** argv) {

    return 0;
}
