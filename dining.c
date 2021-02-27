#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define N 5
#define MIN_SLEEP_TIME 5000
#define MAX_SLEEP_TIME 15000
#define LEFT_NEIGHBOUR (id + N - 1) % N
#define RIGHT_NEIGHBOUR (id + 1) % N

typedef enum {
    THINKING,
    EATING
} philosopher_state;

philosopher_state philosopher_states[N] = {THINKING};
pthread_cond_t philosopher_variables[N];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char running = 1;

void sleep_random_time() {
    int msecs = (rand() % (MAX_SLEEP_TIME - MIN_SLEEP_TIME)) + MIN_SLEEP_TIME;
    struct timespec spec;
    spec.tv_sec = msecs / 1000;
    spec.tv_nsec = (msecs % 1000) * 1000000;
    nanosleep(&spec, &spec);
}

void think(unsigned int id) {
    printf("Philosopher %u thinking...\n", id);
    sleep_random_time();
}

void eat(unsigned int id) {
    printf("Philosopher %u eating!\n", id);
    sleep_random_time();
}

char can_eat(unsigned int id) {
    return philosopher_states[LEFT_NEIGHBOUR] != EATING
            && philosopher_states[RIGHT_NEIGHBOUR] != EATING;
}

void acquire_forks(unsigned int id) {
    pthread_mutex_lock(&mutex);
    while (!can_eat(id))
        pthread_cond_wait(&philosopher_variables[id], &mutex);
    philosopher_states[id] = EATING;
    pthread_mutex_unlock(&mutex);
}

void release_forks(unsigned int id) {
    pthread_mutex_lock(&mutex);
    philosopher_states[id] = THINKING;
    pthread_cond_signal(&philosopher_variables[LEFT_NEIGHBOUR]);
    pthread_cond_signal(&philosopher_variables[RIGHT_NEIGHBOUR]);
    pthread_mutex_unlock(&mutex);
}

void *philosopher(void* id) {
    while (running) {
        think(id);
        acquire_forks(id);
        eat(id);
        release_forks(id);
    }
}

int main() {
    printf("Press ENTER to exit\n");
    srand(time(NULL));
    pthread_t philosophers[N];
    for (unsigned int i = 0; i < N; ++i) {
        pthread_cond_init(&philosopher_variables[i], NULL);
        pthread_create(&philosophers[i], NULL, &philosopher, (void*)i);
    }

    getchar();
    running = 0;
    printf("Waiting for philosophers to exit...\n");
    for (unsigned int i = 0; i < N; ++i)
        pthread_join(philosophers[i], NULL);
    return 0;
}
