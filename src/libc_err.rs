use std::error::Error;
use std::ffi::{c_int, CStr};
use std::fmt::{Display, Formatter};
use libc::strerror;

#[derive(Debug)]
pub struct CError {
    errno: c_int,
}

impl Display for CError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let err_str = unsafe { CStr::from_ptr(strerror(self.errno)) };
        write!(f, "{}", err_str.to_string_lossy())
    }
}

impl Error for CError {}

impl CError {
    pub fn from_errno(errno: c_int) -> Self {
        Self { errno }
    }

    pub fn from_sys_errno() -> Self {
        Self::from_errno(std::io::Error::last_os_error().raw_os_error().expect("unable to find last OS error"))
    }

    pub fn into_err_unless<T>(self, ok_val: T, filter: &[c_int]) -> std::result::Result<T, Self> {
        if filter.contains(&self.errno) {
            return Ok(ok_val);
        }
        self.into_err()
    }

    pub fn into_err<T>(self) -> std::result::Result<T, Self> {
        Err(self)
    }
}