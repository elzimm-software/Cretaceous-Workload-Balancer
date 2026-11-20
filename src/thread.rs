use crate::libc_err::CError;
use libc::{c_void, pthread_detach, pthread_equal, pthread_join, pthread_self};
use libc::{pthread_exit, pthread_t};
use std::ptr::{null, null_mut};

type ThreadError = CError;
type ThreadResult<T> = Result<T, ThreadError>;

/// # Safety
/// Implementors must be `#[repr(C)]`.
pub unsafe trait Runnable {
    fn run(&mut self);
}

/// Safe wrapper for [pthread_t](libc::pthread_t).
/// Uses [Runnable] to enable passing data with the callback.
pub struct Thread {
    pthread: pthread_t,
}

extern "C" fn pthread_callback<R>(ptr: *mut c_void) -> *mut c_void
where
    R: Runnable,
{
    let runner = unsafe { &mut *(ptr as *mut R) };
    runner.run();
    unsafe { pthread_exit(null_mut()) };
}

impl Thread {
    pub fn create<R>(mut runner: R) -> ThreadResult<Self>
    where
        R: Runnable,
    {
        let mut t = pthread_t::default();
        let rv = unsafe {
            libc::pthread_create(
                &mut t,
                null(),
                pthread_callback::<R>,
                &mut runner as *mut R as *mut c_void,
            )
        };

        if rv != 0 {
            return ThreadError::from_errno(rv).into_err();
        }
        Ok(Self { pthread: t })
    }

    pub fn join(self) -> ThreadResult<()> {
        if unsafe { pthread_equal(pthread_self(), self.pthread) } == 0 {
            return Ok(()); // TODO: should this actually return Ok or be its own error
        }
        let rv = unsafe { pthread_join(self.pthread, null_mut()) };
        if rv != 0 {
            return ThreadError::from_errno(rv).into_err();
        }
        Ok(())
    }
}

impl Drop for Thread {
    fn drop(&mut self) {
        let _ = unsafe { pthread_detach(self.pthread) };
    }
}
