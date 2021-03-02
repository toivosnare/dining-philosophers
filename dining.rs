use rand::Rng;
use std::thread;
use std::time::Duration;
use std::sync::{Arc, Mutex, Condvar};
use std::io::stdin;

const N: usize = 5; // Number of philosophers (and forks).
const MIN_SLEEP_TIME: u32 = 5000;
const MAX_SLEEP_TIME: u32 = 15000;

#[derive(Copy,Clone,PartialEq)]
enum PhilosopherState {
    Thinking,
    Eating
}
type PID = usize; // Philosopher ID (usize can index a vector).

fn sleep_random_time() {
    let msecs = rand::thread_rng().gen_range(MIN_SLEEP_TIME..MAX_SLEEP_TIME);
    thread::sleep(Duration::from_millis(msecs.into()));
}

fn think(id: PID) {
    println!("Philosopher {} thinking...", id);
    sleep_random_time();
}

fn eat(id: PID) {
    println!("Philosopher {} eating!", id);
    sleep_random_time();
}

fn left_neighbour_of(id: PID) -> PID {
    (id + N - 1) % N
}

fn right_neighbour_of(id: PID) -> PID {
    (id + 1) % N
}

fn acquire_forks(id: PID, arc: Arc<(Mutex<Vec<PhilosopherState>>, Vec<Condvar>)>) {
    let (mutex, condvars) = &*arc;
    let condvar = &condvars[id];
    let mut guard = condvar.wait_while(mutex.lock().unwrap(), |states| {
        states[left_neighbour_of(id)] == PhilosopherState::Eating
        || states[right_neighbour_of(id)] == PhilosopherState::Eating
    }).unwrap();
    guard[id] = PhilosopherState::Eating;
}

fn release_forks(id: PID, arc: Arc<(Mutex<Vec<PhilosopherState>>, Vec<Condvar>)>) {
    let (mutex, condvars) = &*arc;
    let mut guard = mutex.lock().unwrap();
    guard[id] = PhilosopherState::Thinking;
    condvars[left_neighbour_of(id)].notify_one();
    condvars[right_neighbour_of(id)].notify_one();
}

fn philosopher(id: PID, arc: Arc<(Mutex<Vec<PhilosopherState>>, Vec<Condvar>)>) {
    loop {
        think(id);
        acquire_forks(id, arc.clone());
        eat(id);
        release_forks(id, arc.clone());
    }
}

fn main() {
    let mut states = Vec::with_capacity(N);
    let mut condvars = Vec::with_capacity(N);
    for _ in 0..N {
        states.push(PhilosopherState::Thinking);
        condvars.push(Condvar::new());
    }
    let arc = Arc::new((Mutex::new(states), condvars));

    println!("Press ENTER to exit");
    for id in 0..N {
        let arc = arc.clone();
        thread::spawn(move || { philosopher(id, arc) });
    }

    let mut line = String::new();
    stdin().read_line(&mut line).unwrap();
}
