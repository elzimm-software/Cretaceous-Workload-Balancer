use std::cell::Cell;
use std::cmp::PartialEq;
use crate::libc_err::CError;
use libc::{sem_init, sem_t, sem_trywait, sem_wait, EAGAIN, sem_post};
use std::mem::MaybeUninit;
use std::ops::Deref;
use std::sync::Arc;
use crate::atomic_semaphore::TryWaitResult::{Acquired, NotAcquired};

pub type SemID = u32;

pub type SemError = CError;
pub type SemResult<T> = Result<T, SemError>;

pub enum TryWaitResult {
    Acquired,
    NotAcquired
}

impl TryWaitResult {
    pub fn is_acquired(&self) -> bool {
        match self {
            Acquired => true,
            _ => false,
        }
    }
}

pub struct AtomicSemaphore {
    sem: Arc<Cell<sem_t>>,
    id: SemID,
}

impl AtomicSemaphore {
    pub fn init<T>(id: T) -> SemResult<Self>
    where
        T: Into<SemID>,
    {
        let mut s: MaybeUninit<sem_t> = MaybeUninit::zeroed();
        if unsafe { sem_init(s.as_mut_ptr(), 0, 1) } != 0 {
            return SemError::from_sys_errno().into_err();
        }
        Ok(Self {
            sem: Arc::new(Cell::new(unsafe { s.assume_init() })),
            id: id.into(),
        })
    }

    pub fn wait(&mut self) -> SemResult<()> {
        if unsafe {sem_wait(self.sem.as_ptr())} != 0 {
            return SemError::from_sys_errno().into_err()
        }
        Ok(())
    }

    pub fn try_wait(&mut self) -> SemResult<TryWaitResult> {
        if unsafe {sem_trywait(self.sem.as_ptr())} != 0 {
            return SemError::from_sys_errno().into_err_unless(NotAcquired, &[EAGAIN]);
        }
        Ok(Acquired)
    }

    pub fn post(&mut self) -> SemResult<()> {
        if unsafe {sem_post(self.sem.as_ptr())} != 0 {
            return SemError::from_sys_errno().into_err();
        }
        Ok(())
    }
}
