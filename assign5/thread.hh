
#pragma once

#include <queue>
#include <functional>

#include "stack.hh"

class Thread {
public:
    /* Thread constructor - initializes a new thread that will run
     * the specified function (which must take in no arguments and return
     * nothing).  This initializes all needed internal state,
     * and then adds the thread to the back of the ready queue.
     * If `main` returns, the thread is terminated just as if it had 
     * invoked `Thread::exit`.
     */
    Thread(std::function<void()> main);

    /* Schedules a thread to run when the CPU is available by adding it to
     * the back of the ready queue.  (The thread should be ready, 
     * and not blocked, when this function is called.)
     */
    void schedule();

    /* Stop executing the current thread and switch to the next ready
     * thread. The caller must make sure that the current thread has
     * been scheduled or kept track of in some other way before calling
     * this; otherwise it will never run again.  Must be invoked 
     * with interrupts disabled.  If there are no more ready threads,
     * this method terminates the entire program by calling std::exit(0).
     */
    static void redispatch();

    /* Returns a pointer to the currently-running thread, or nullptr if no
     * thread is currently running.
     */
    static Thread *current();

    /* Schedules the current thread by calling schedule() on the current thread,
     * and then switches to the next scheduled thread by calling redispatch().
     * The current thread remains ready, but other threads get a 
     * chance to execute.
     */
    static void yield();

    /* This terminates/deletes the current running thread and then
     * continues running the next ready thread (if any).  It never fully
     * returns, because as part of its implementation it cleans up the current
     * thread and then switch to another thread, which will never switch back
     *  to the current (deleted) one.
     */
    [[noreturn]] static void exit();

    /* Initializes preemptive threading.  If this function is called
     * once at the start of the program, it then preempts the
     * currently running thread every usec microseconds.  If this method
     * is not called, then your dispatcher should be non-preemptive 
     * (threads run until they either yield or exit).
     */
    static void preempt_init(std::uint64_t usec = 100'000);

private:
    // A list of threads that are ready to execute. The "static" keyword
    // means that there is only a single instance of this variable, which
    // is shared across all Thread instances.
    static std::queue<Thread*> ready_;
    
    // It's important that the destructor is private: threads can only
    // be deleted when they invoke exit.
    ~Thread() {};

    /* The function that is called when a thread is run for the first time.
     * The thread being run is passed in as the first parameter.  This function
     * should do any necessary set up, then run this thread's function, and
     * finally exit the thread if it returns from its function.
     */
    static void thread_start(Thread *t);

    // Fill in other fields and/or methods that you need.
    std :: function<void()> func;
    Stack *stack;
    static bool has_running_thread;
    static Thread *curr_;
    static Stack *prev_stack;
};
