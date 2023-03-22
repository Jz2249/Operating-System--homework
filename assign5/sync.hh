
#pragma once


#include "thread.hh"

// A standard mutex providing mutual exclusion.
class Mutex {
public:
    // This is the constructor for `Mutex` objects. It has no arguments.
    Mutex();

    /* Locks the `Mutex`. If the `Mutex` is currently locked, 
     * blocks the calling thread until the `Mutex` is unlocked. 
     * If there are multiple blocked threads, they return 
     * from `lock` in FIFO order (in other words, the first 
     * thread to block is the first thread to get the lock). 
     */
    void lock();

    /* If there are threads waiting to lock the `Mutex`, 
     * passes ownership of the `Mutex` to the first waiting 
     * thread and wakes it up. If there are no threads waiting, 
     * releases the lock.
     */
    void unlock();

    // Returns true if the lock is held by the current thread, false otherwise
    bool mine();

    // These lines prevent Mutexes from being copied
    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

private:
    // You must define appropriate variables for this object
    bool is_lock;
    std::queue<Thread*> blockQueue;
    Thread* lock_owner;
};

// A condition variable; methods are similar to std::condition_variable_any
// except that wait takes a Mutex as an argument rather than a std::unique_lock.
class Condition {
public:
    // This is the constructor for `Condition` objects.  It takes no arguments.
    Condition();

    /* Releases the Mutex and blocks the thread until `notify_one` or
     * `notify_all` has been invoked, then reacquires the Mutex. The
     * caller must have locked `m` before calling this method.
     */
    void wait(Mutex &m);

    /* If any threads are blocked on the condition variable, wakes up the one
     * that has been blocked the longest. If no threads are blocked, then this
     * method does nothing. The caller must have locked the associated
     * Mutex before calling this method.
     */
    void notify_one();	     

    /* Wakes up all of the threads currently blocked on the condition variable.
     * The caller must have locked the associated Mutex before 
     * calling this method.
     */
    void notify_all();

private:
    // You must define appropriate variables for this object
    std::queue<Thread*> blockQueue_;
};
