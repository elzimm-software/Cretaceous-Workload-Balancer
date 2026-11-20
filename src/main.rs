use std::process::ExitCode;
use crate::workload_balancer::WorkloadBalancer;

mod thread;
mod atomic_semaphore;
mod libc_err;
mod atomic_rng;
mod workload_balancer;

fn main() -> ExitCode {
    const N_THREADS: usize = 5;
    const N_SEMAPHORES: usize = 3;
    WorkloadBalancer::new(N_THREADS, N_SEMAPHORES).run()
}
