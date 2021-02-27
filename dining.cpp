#include <array>
#include <condition_variable>
#include <mutex>
#include <random>
#include <thread>
#include <chrono>
#include <cstdio>

constexpr unsigned int N = 5; // Number of philosophers (and forks).
constexpr unsigned int MIN_SLEEP_TIME = 5000;
constexpr unsigned int MAX_SLEEP_TIME = 15000;

enum PHILOSOPHER_STATE {
    THINKING,
    EATING
};

std::array<PHILOSOPHER_STATE, N> philosopher_states = {THINKING};
std::array<std::condition_variable, N> philosopher_variables;
std::mutex mutex;
bool running = true;

void sleep_random_time() {
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int> distribution(MIN_SLEEP_TIME, MAX_SLEEP_TIME);
    std::this_thread::sleep_for(std::chrono::milliseconds(distribution(generator)));
}

void think(unsigned int id) {
    printf("Philosopher %u thinking...\n", id);
    sleep_random_time();
}

void eat(unsigned int id) {
    printf("Philosopher %u eating!\n", id);
    sleep_random_time();
}

inline unsigned int left_neighbour_of(unsigned int id) {
    return (id + N - 1) % N;
}

inline unsigned int right_neighbour_of(unsigned int id) {
    return (id + 1) % N;
}

bool can_eat(unsigned int id) {
    return philosopher_states[left_neighbour_of(id)] != EATING
            && philosopher_states[right_neighbour_of(id)] != EATING;
}

void acquire_forks(unsigned int id) {
    std::unique_lock<std::mutex> lock(mutex);
    philosopher_variables[id].wait(lock, [id](){ return can_eat(id); });   
    philosopher_states[id] = EATING;
}

void release_forks(unsigned int id) {
    std::unique_lock<std::mutex> lock(mutex);
    philosopher_states[id] = THINKING;
    philosopher_variables[left_neighbour_of(id)].notify_one();
    philosopher_variables[right_neighbour_of(id)].notify_one();
}

void philosopher(unsigned int id) {
    while (running) {
        think(id);
        acquire_forks(id);
        eat(id);
        release_forks(id);
    }
}

int main() {
    printf("Press ENTER to exit\n");
    std::array<std::thread, N> philosophers;
    for (unsigned int id = 0; id < N; ++id)
        philosophers[id] = std::thread(philosopher, id);

    getchar();
    running = false;
    printf("Waiting for philosophers to exit...\n");
    for (std::thread &p : philosophers)
        p.join();
    return 0;
}
