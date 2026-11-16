
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <memory>
#include <random>
#include <cstring>
#include <syncstream>

#define CRETBAL_DO_SLEEP
#define MILLI_FACTOR 1000000

constexpr size_t N_THREADS = 5;
constexpr size_t N_SEMAPHORES = 3;

static std::uniform_int_distribution<int> dist;
using P = decltype(dist)::param_type;

static int roll(std::mt19937& rng, int n) {
    return dist(rng, P(1, n));
}

static void shuffle_n(sem_t** a, size_t n, size_t m, std::mt19937& rng) {
    for (size_t i = 0; i < m; i += 1) {
        std::uniform_int_distribution<size_t> d(i, n - 1);
        std::swap(a[i], a[d(rng)]);
    }
}

class ThreadManager {
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
        int work = 10;
        sem_t* held[N_SEMAPHORES]{nullptr};
        while (work > 0) {
            bool all_held = false;
            int n_sems = roll(*gen, N_SEMAPHORES);
            std::osyncstream(std::cout) << id << " needs " << n_sems << "\n";
            shuffle_n(sem, N_SEMAPHORES, n_sems, *gen);
            while (!all_held) {
                all_held = true; // assume we got them all
                for (int i = 0; i < n_sems; i += 1) {
                    if (sem_trywait(sem[i]) == -1) {
                        for (int j = 0; i < N_SEMAPHORES && held[j] != nullptr; j += 1) {
                            std::osyncstream(std::cout) << id << " released " << held[j] << "\n";
                            sem_post(held[j]);
                            held[j] = nullptr;
                        }
                        all_held = false; // if we didn't, indicate such
#ifdef CRETBAL_DO_SLEEP
                        const timespec duration = {.tv_sec = 0, .tv_nsec = (roll(*gen, 11) - 1) * MILLI_FACTOR};
                        std::osyncstream(std::cout) << id << " sleeping for " << duration.tv_nsec << "\n";
                        nanosleep(&duration, nullptr);
#endif
                        break;
                    }
                    held[i] = sem[i];
                    std::osyncstream(std::cout) << id << " got " << sem[i] << "\n"; // yeah we're using memory addresses as ids, sue me
                }
            }
            std::osyncstream(std::cout) << id << " has " << work << " left" << "\n";
            work -= 1;
            for (int i = 0; i < N_SEMAPHORES && held[i] != nullptr; i += 1) {
                std::osyncstream(std::cout) << id << " released " << held[i] << "\n";
                sem_post(held[i]);
                held[i] = nullptr;
            }
#ifdef CRETBAL_DO_SLEEP
            const timespec duration = {.tv_sec = 0, .tv_nsec = (roll(*gen, 11) - 1) * MILLI_FACTOR};
            std::osyncstream(std::cout) << id << " sleeping for " << duration.tv_nsec << "\n";
            nanosleep(&duration, nullptr);
#endif
        }
        pthread_exit(nullptr);
    }
public:
    pthread_t t[N_THREADS]{};
    int ret[N_THREADS]{};
    sem_t* sem[N_SEMAPHORES]{};

    std::mt19937 gen;

    ThreadManager() {
        std::random_device rd;
        gen = std::mt19937(rd());
    }

    void run() {
        for (int i = 0; i < N_SEMAPHORES; i += 1) {
            sem[i] = new sem_t;
            sem_init(sem[i], 0, 1);
        }
        char next_id = 'A';
        for (int i = 0; i < N_THREADS; i += 1) {
            auto** these_sems = new sem_t* [N_SEMAPHORES];
            std::memcpy(these_sems, sem, N_SEMAPHORES * sizeof(sem_t*));
            ret[i] = pthread_create(&t[i], nullptr, thread_do_work, new thread_args_t {.sem = these_sems, .id = next_id, .gen = &gen});
            next_id += 1;
        }
        for (int i = 0; i < N_THREADS; i += 1) {
            pthread_join(t[i], nullptr);
        }
    }
};

int main(int argc, char** argv) {
    ThreadManager t;
    t.run();
    return 0;
}
