// https://mmomtchev.medium.com/spinlock-vs-mutex-performance-3f73de53a460

// keypoint: do not use spinlocks in userland unless you know what you are doing.
// https://www.realworldtech.com/forum/?threadid=189711&curpostid=189723

// https://rigtorp.se/spinlock/
// tas_lock: test-and-set lock
// ttas_lock: test and test-and-set lock: optimized for the uncontended case. First it tries to acquire the lock, if that
// fails it spins waiting for the lock to be released.
