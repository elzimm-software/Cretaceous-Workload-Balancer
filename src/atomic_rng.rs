use rand::RngExt;
use rand::distr::uniform::SampleUniform;
use rand::rngs::SmallRng;
use std::sync::{Arc, Mutex};

pub struct AtomicRng {
    rng: Arc<Mutex<SmallRng>>,
}

impl Clone for AtomicRng {
    fn clone(&self) -> Self {
        Self {
            rng: self.rng.clone(),
        }
    }
}

impl AtomicRng {
    pub fn new() -> Self {
        Self {
            rng: Arc::new(Mutex::new(rand::make_rng())),
        }
    }

    pub fn rand_in_range<T, U, L>(&mut self, lower: U, upper: L) -> T
    where
        T: PartialOrd + SampleUniform,
        U: Into<T>,
        L: Into<T>,
    {
        self.rng
            .lock()
            .unwrap()
            .random_range(lower.into()..upper.into())
    }

    fn random_indexes(&mut self, size: impl Into<usize>, n: impl Into<usize>) -> Vec<usize> {
        let n = n.into();
        let size = size.into();
        let mut rng = self.rng.lock().unwrap();
        (0..n)
            .into_iter()
            .map(|i| rng.random_range(i..size))
            .collect()
    }

    pub fn shuffle_n<V, T, N>(&mut self, v: V, n: N) -> Vec<T>
    where
        V: AsRef<[T]>,
        N: Into<usize>,
        T: Clone,
    {
        let size = v.as_ref().len();
        let n = n.into().min(size);
        let js = self.random_indexes(size, n);
        let mut out = v.as_ref().to_vec();
        for (i, j) in js.into_iter().enumerate() {
            out.swap(i, j);
        }
        out.truncate(n);
        out
    }
}
