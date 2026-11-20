use crate::atomic_rng::AtomicRng;
use crate::atomic_semaphore::AtomicSemaphore;
use crate::thread::{Runnable, Thread};
use libc::timespec;
use libc::{c_long, nanosleep};
use std::process::ExitCode;
use std::ptr::null_mut;

const INITIAL_WORK: u32 = 10;

#[repr(C)]
struct Runner {
    id: char,
    sems: Vec<AtomicSemaphore>,
    rng: AtomicRng,
    work: u32,
}

impl Runner {
    fn new<C, W, S>(id: C, initial_work: W, sems: S, rng: AtomicRng) -> Self
    where
        C: Into<char>,
        W: Into<u32>,
        S: AsRef<[AtomicSemaphore]>,
    {
        Self {
            id: id.into(),
            work: initial_work.into(),
            sems: sems.as_ref().to_vec(),
            rng,
        }
    }

    fn post_all<V>(&mut self, mut held: V)
    where
        V: AsMut<[AtomicSemaphore]>,
    {
        for s in held.as_mut() {
            self.post_sem(s);
        }
    }

    fn sleep_rand(&mut self) {
        {
            #[cfg(feature = "do_sleep")]
            {
                const MILLIS: u32 = 1_000_000;
                let nanos = self.rng.rand_in_range::<u32, u32, u32>(0, 10) * MILLIS;
                let duration: timespec = timespec {
                    tv_nsec: 0,
                    tv_sec: nanos as c_long,
                };
                #[cfg(not(feature = "terse_logs"))]
                {
                    println!("{} sleeping for {}", self.id, nanos);
                }

                unsafe {
                    nanosleep((&duration) as *const timespec, null_mut());
                }
            }
        }
    }

    fn take_sem(&self, s: &AtomicSemaphore) -> bool {
        if !s.try_wait().unwrap().is_acquired() {
            return false;
        }

        const VERB: &str = if cfg!(feature = "terse_logs") {
            ">"
        } else {
            "got"
        };

        println!("{} {} {}", self.id, VERB, s.id());
        true
    }

    fn post_sem(&self, s: &AtomicSemaphore) {
        s.post().unwrap();

        const VERB: &str = if cfg!(feature = "terse_logs") {
            "<"
        } else {
            "released"
        };
        println!("{} {} {}", self.id, VERB, s.id());
    }
}

unsafe impl Runnable for Runner {
    fn run(&mut self) {
        let n_sems = self.sems.len();
        let mut held: Vec<AtomicSemaphore> = Vec::with_capacity(n_sems);
        while self.work > 0 {
            let mut all_held = false;
            let needed_count: usize = self.rng.rand_in_range(1_usize, n_sems);
            #[cfg(not(feature = "terse_logs"))]
            {
                println!("{} needs {} semaphores", self.id, needed_count);
            }
            let sems_needed = self.rng.shuffle_n(self.sems.as_slice(), needed_count);
            while !all_held {
                all_held = true;
                for s in &sems_needed {
                    if self.take_sem(s) {
                        held.push(s.clone());
                    } else {
                        self.post_all(held.as_mut_slice());
                        held.clear();
                        all_held = false;
                        self.sleep_rand();
                    }
                }
            }
            self.work -= 1;
            println!("{} has {} left", self.id, self.work);
            self.post_all(held.as_mut_slice());
            held.clear();
            self.sleep_rand()
        }
    }
}

pub struct WorkloadBalancer {
    n_threads: usize,
    n_semaphores: usize,
    threads: Vec<Thread>,
    semaphores: Vec<AtomicSemaphore>,
    rng: AtomicRng,
}

impl WorkloadBalancer {
    pub fn new<T, S>(n_threads: T, n_semaphores: S) -> Self
    where
        T: Into<usize>,
        S: Into<usize>,
    {
        let n_threads = n_threads.into();
        let n_semaphores = n_semaphores.into();
        Self {
            n_threads,
            n_semaphores,
            threads: Vec::with_capacity(n_threads),
            semaphores: (0..n_semaphores)
                .map(|i| AtomicSemaphore::init(i as u32).unwrap())
                .collect(),
            rng: AtomicRng::new(),
        }
    }

    pub fn run(mut self) -> ExitCode {
        const INITIAL_ID: u32 = 'A' as u32;
        for i in 0..self.n_threads {
            self.threads.push(
                Thread::create(Runner::new(
                    char::from_u32(INITIAL_ID + i as u32).unwrap(),
                    INITIAL_WORK,
                    self.semaphores.clone(),
                    self.rng.clone(),
                ))
                .unwrap(),
            )
        }
        for t in self.threads {
            t.join().unwrap();
        }
        ExitCode::SUCCESS
    }
}
