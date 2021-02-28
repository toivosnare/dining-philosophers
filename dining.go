package main

import (
    "sync"
    "math/rand"
    "time"
    "fmt"
)

const N = 5 // Number of philosophers (and forks).
const MIN_SLEEP_TIME = 5000
const MAX_SLEEP_TIME = 15000

type philosopher_state uint
const (
    THINKING philosopher_state = 0
    EATING philosopher_state = 1
)

var philosopher_states [N]philosopher_state
var philosopher_variables [N]*sync.Cond
var mutex sync.Mutex
var running bool = true

func sleep_random_time() {
    msecs := rand.Intn(MAX_SLEEP_TIME - MIN_SLEEP_TIME) + MIN_SLEEP_TIME
    time.Sleep(time.Duration(msecs) * time.Millisecond)
}

func think(id uint) {
    fmt.Printf("Philosopher %d thinking...\n", id)
    sleep_random_time()
}

func eat(id uint) {
    fmt.Printf("Philosopher %d eating!\n", id)
    sleep_random_time()
}

func left_neighbour_of(id uint) uint {
    return (id + N - 1) % N
}

func right_neighbour_of(id uint) uint {
    return (id + 1) % N
}

func can_eat(id uint) bool {
    return philosopher_states[left_neighbour_of(id)] != EATING &&
            philosopher_states[right_neighbour_of(id)] != EATING
}

func acquire_forks(id uint) {
    mutex.Lock()
    defer mutex.Unlock()
    for !can_eat(id) {
        philosopher_variables[id].Wait()
    }
    philosopher_states[id] = EATING
}

func release_forks(id uint) {
    mutex.Lock()
    defer mutex.Unlock()
    philosopher_states[id] = THINKING
    philosopher_variables[left_neighbour_of(id)].Signal()
    philosopher_variables[right_neighbour_of(id)].Signal()
}

func philosopher(id uint, wg *sync.WaitGroup) {
    defer wg.Done()
    for running {
        think(id)
        acquire_forks(id)
        eat(id)
        release_forks(id)
    }
}

func main() {
    fmt.Println("Press ENTER to exit")
    rand.Seed(time.Now().UnixNano())
    var wg sync.WaitGroup
    wg.Add(N)
    for i := uint(0); i < N; i++ {
        philosopher_states[i] = THINKING
        philosopher_variables[i] = sync.NewCond(&mutex)
        go philosopher(i, &wg)
    }

    fmt.Scanln()
    running = false
    fmt.Println("Waiting for philosophers to exit...")
    wg.Wait()
}
