/*
 * This file tests the code in thread.cc and sync.cc. Run this program
 * with one or more test names as arguments (see main below for the
 * names of existing tests). Please report any bugs to the course staff.
 *
 * Note that passing these tests doesn't guarantee that your code is correct
 * or meets the specifications given, but hopefully it's at least pretty
 * close.
 */

#include <atomic>
#include <cstring>
#include <iostream>

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "thread.hh"
#include "sync.hh"
#include "timer.hh"

/**
 * This function doesn't return until either a context switch has
 * occurred or many seconds have elapsed. Context switches are detected
 * by polling a shared variable holding an id for the most recent thread
 * to run: when this is no longer equal to our_id, it means we must have
 * been preempted and then later started running again.
 */
void
wait_for_switch(std::atomic<int> *most_recent, int our_id)
{
    struct timeval start, current;

    if (gettimeofday(&start, nullptr) != 0) {
        std::cout << "gettimeofday failed: " << strerror(errno) << std::endl;
    }
    while (most_recent->load() == our_id) {
        if (gettimeofday(&current, nullptr) != 0) {
            std::cout << "gettimeofday failed: " << strerror(errno) << std::endl;
        }
        if ((1'000'000*(current.tv_sec - start.tv_sec)
                    + (current.tv_usec - start.tv_usec)) > 5'000'000) {
            std::cout <<  "5 seconds elapsed with no preemption" << std::endl;
            break;
        }
    }
}

//--------------------------------------------------------------------------

void
enter_thread()
{
    std::cout << "Entered thread" << std::endl;
    
    // Thread destruction may not have been implemented yet, so exit
    // the test application.
    exit(0);
}

void
enter_test()
{
    // See if we can create a thread and enter its main function.
    new Thread(enter_thread);
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void ping_pong_thread1()
{
    std::cout << "Thread 1 started; now yielding" << std::endl;
    Thread::yield();
    std::cout << "Thread 1 woke up; yielding again" << std::endl;
    Thread::yield();
    std::cout << "Thread 1 woke up again; yielding one more time" << std::endl;
    Thread::yield();
    std::cout << "Thread 1 woke up for the last time" << std::endl;
    exit(0);
}

void ping_pong_thread2()
{
    std::cout << "Thread 2 started; now yielding" << std::endl;
    Thread::yield();
    std::cout << "Thread 2 woke up; yielding again" << std::endl;
    Thread::yield();
    std::cout << "Thread 2 woke up again; yielding for the last time"
            << std::endl;
    Thread::yield();
}

void
ping_pong_test()
{
    // This test creates two threads, which yield back and forth
    // to each other several times.
    new Thread(ping_pong_thread1);
    new Thread(ping_pong_thread2);
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void round_robin_thread(int thread_num)
{
    for (int i = 0; i < 3; i++) {
        std::cout << "Thread " << thread_num << " woke up" << std::endl;
        Thread::yield();
    }
    exit(0);
}

void
round_robin_test()
{
    // This test creates many threads, which take turns running.
    for (int i = 0; i < 5; i++) {
        // The notation in the parentheses below is called a "closure".
        // You don't need to learn closures for this class, but they
        // are very useful in C++. This closure causes the round_robin_thread
        // function to be invoked with an argument containing i when the
        // thread starts running. More precisely, it defines an unnamed
        // function that takes no arguments; that function is passed to the
        // Thread constructor, and it invokes round_robin_thread. The
        // notation "[i]" is called a "capture": it records the value of
        // the loop variable i and makes that value accessible as a local
        // variable (also named i) in the unnamed function.
        new Thread([i] {round_robin_thread(i);});
    }
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

// This (awkward) global variable is used for passing information between
// block_thread1 and block_thread2. This functionality could be implemented
// more cleanly with closures, but we're trying to avoid use of closures,
// since they aren't an official part of the class.

Thread *thread = nullptr;

void
block_thread1()
{
    thread = Thread::current();
    std::cout << "Thread 1 running; about to block" << std::endl;
    {
        IntrGuard ig;
        Thread::redispatch();
    }
    intr_enable(true);
    std::cout << "Thread 1 thread woke up from block" << std::endl;
    exit(0);
}

void
block_thread2()
{
    std::cout << "Thread 2 running; yielding to self" << std::endl;
    Thread::yield();
    std::cout << "Thread 2 woke up (thread 1 still blocked); yielding again"
                << std::endl;
    Thread::yield();
    std::cout << "Thread 2 woke up again; waking thread 1" << std::endl;
    thread->schedule();
    std::cout << "Thread 2 yielding for the last time" << std::endl;
    Thread::yield();
}

void
block_test()
{
    // This test checks to see whether a thread can block and then
    // be woken up again.
    new Thread(block_thread1);
    new Thread(block_thread2);
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
exit_thread1()
{
    std::cout << "Thread 1 running; yielding" << std::endl;
    Thread::yield();
    std::cout << "Thread 1 thread woke up; yielding again" << std::endl;
    Thread::yield();
    std::cout << "Thread 1 thread woke up for the last time" << std::endl;
    exit(0);
}

void
exit_thread2()
{
        std::cout << "Thread 2 running; exiting" << std::endl;
        Thread::exit();
}

void
exit_thread3()
{
        std::cout << "Thread 3 running; returning" << std::endl;
}

void
exit_test()
{
    // This test checks to be sure that a thread exits properly and returns
    // control to other threads.
    new Thread(exit_thread1);
    new Thread(exit_thread2);
    new Thread(exit_thread3);
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void exit2_thread()
{
        std::cout << "Thread running; exiting" << std::endl;
        Thread::current()->schedule();
        Thread::exit();
        
        // The code below should never get executed.
        std::cout << "Thread came back to life; looks like the Thread "
                "destructor didn't make sure it's not on the ready queue?"
                << std::endl;
        exit(0);
    
}

void
exit2_test()
{
    // This test checks to be sure that the thread destructor removes
    // the thread from the ready queue, if it's present.
    new Thread(exit2_thread);
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
stack_thread(int thread_num)
{
    std::cout << "Thread " << thread_num << " woke up (" << Stack::num_stacks_
            << " stacks); exiting" << std::endl;
    Thread::exit();
}

void
stack_test()
{
    // This test checks for proper stack deletion. It creates several
    // threads, which exit in the order created. After the first exit,
    // no stacks should be deleted, but one stack should be deleted
    // in each subsequent exit. This test also makes sure that the
    // Thread class exits the application when there are no longer
    // any ready threads.
    for (int i = 0; i < 5; i++) {
        new Thread([i] {stack_thread(i);});
        std::cout << "Created thread " << i << " (" << Stack::num_stacks_
                << " stacks)" << std::endl;
    }
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
preempt_thread1(std::atomic<int> &most_recent)
{
    for (int i = 0; i < 3; i++) {
        if (!intr_enabled())
            std::cout << "Interrupts are disabled!" << std::endl;
        most_recent.store(1);
        std::cout << "thread 1 now running" << std::endl;
        wait_for_switch(&most_recent, 1);
    }
    most_recent.store(1);
    std::cout << "thread 1 now running; exiting" << std::endl;
}

void
preempt_thread2(std::atomic<int> &most_recent)
{
    for (int i = 0; i < 3; i++) {
        if (!intr_enabled())
            std::cout << "Interrupts are disabled!" << std::endl;
        most_recent.store(2);
        std::cout << "thread 2 now running" << std::endl;
        wait_for_switch(&most_recent, 2);
    }
    most_recent.store(2);
    std::cout << "thread 2 now running; exiting" << std::endl;
}

void
preempt_test()
{
    // This test starts two threads and sees whether preemption will
    // occur (so that the threads execute in round-robin fashion)
    // even though neither of the threads yields or blocks.
    
    // Id of most recent thread to start a time slice.
    std::atomic<int> most_recent(-1);
    Thread::preempt_init(100'000);

    new Thread([&most_recent]{preempt_thread1(most_recent);});
    new Thread([&most_recent]{preempt_thread2(most_recent);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
basic_thread1(Mutex &m)
{
    m.lock();
    std::cout << "thread 1 yielding while holding lock" << std::endl;
    Thread::yield();
    std::cout << "thread 1 yielding again while holding lock" << std::endl;
    Thread::yield();
    std::cout << "thread 1 releasing lock then trying to reacquire" << std::endl;
    m.unlock();
    m.lock();
    std::cout << "thread 1 reacquired lock" << std::endl;
}

void
basic_thread2(Mutex &m)
{
    std::cout << "thread 2 attempting to lock" << std::endl;
    m.lock();
    std::cout << "thread 2 acquired lock; now unlocking" << std::endl;
    m.unlock();
}

void
mutex_basic_test()
{
    // This test ping-pongs a Mutex back and forth between 2 threads
    // to make sure that threads can properly block on a Mutex and
    // then wake up again.
    Mutex m;

    new Thread([&m] {basic_thread1(m);});
    new Thread([&m] {basic_thread2(m);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void many_threads_control_thread(Mutex &m1, Mutex &m2)
{
    m1.lock();
    m2.lock();
    std::cout << "control_thread yielding to other threads while holding locks"
            << std::endl;
    Thread::yield();
    std::cout << "control_thread unlocking m1 then yielding" << std::endl;
    m1.unlock();
    Thread::yield();
    std::cout << "control_thread yielding again" << std::endl;
    Thread::yield();
    std::cout << "control_thread yielding again" << std::endl;
    Thread::yield();
    std::cout << "control_thread unlocking m2 then trying to lock m1" << std::endl;
    m2.unlock();
    m1.lock();
    std::cout << "control_thread unlocking m1, then trying to reacquire m2" << std::endl;
    m1.unlock();
    m2.lock();
    std::cout << "control_thread reaqcuired m2" << std::endl;
}

void
many_threads_thread(Mutex &m1, Mutex &m2, int thread_num)
{
    std::cout << "thread " << thread_num << " locking m1" << std::endl;
    m1.lock();
    std::cout << "thread " << thread_num << " unlocking m1, locking m2"
            << std::endl;
    m1.unlock();
    m2.lock();
    std::cout << "thread " << thread_num << " locked m2; unlocking and exiting"
            << std::endl;
    m2.unlock();
}

void
mutex_many_threads_test()
{
    // In this test several threads all wait for first one lock then
    // another. A controlling thread owns both locks, and releases
    // them one at a time for the other threads to acquire.
    
    Mutex m1, m2;

    new Thread([&m1, &m2] {many_threads_control_thread(m1, m2);});
    for (int i = 0; i < 3; i++) {
        new Thread([&m1, &m2, i] {many_threads_thread(m1, m2, i);});
    }
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
mine_thread1(Mutex &m)
{
    std::cout << std::boolalpha;
    std::cout << "thread 1 calling mine() before acquiring lock: returns " 
              << m.mine() << std::endl;
    m.lock();
    std::cout << "thread 1 calling mine() after acquiring lock: returns " 
              << m.mine() << std::endl;
    std::cout << "thread 1 yielding while holding lock" << std::endl;
    Thread::yield();
    std::cout << "thread 1 yielding again while holding lock" << std::endl;
    Thread::yield();
    std::cout << "thread 1 releasing lock then trying to reacquire" 
              << std::endl;
    m.unlock();
    std::cout << "thread 1 calling mine() after releasing lock: returns " 
              << m.mine() << std::endl;
    m.lock();
    std::cout << "thread 1 reacquired lock" << std::endl;
    std::cout << "thread 1 calling mine() after acquiring lock: returns " 
              << m.mine() << std::endl;
}

void
mine_thread2(Mutex &m)
{
    std::cout << std::boolalpha;
    std::cout << "thread 2 calling mine() before acquiring lock: returns " 
              << m.mine() << std::endl;
    std::cout << "thread 2 attempting to lock" << std::endl;
    m.lock();
    std::cout << "thread 2 acquired lock" << std::endl;
    std::cout << "thread 2 calling mine() after acquiring lock: returns " 
              << m.mine() << std::endl;
    std::cout << "thread 2 now unlocking" << std::endl;
    m.unlock();
    std::cout << "thread 2 calling mine() after releasing lock: returns " 
              << m.mine() << std::endl;

}

void
mutex_mine_test()
{
    // This test ping-pongs a Mutex back and forth between 2 threads
    // to make sure that the mutex owner is properly recorded.
    Mutex m;    
    new Thread([&m] {mine_thread1(m);});
    new Thread([&m] {mine_thread2(m);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
cond_basic_thread1(Mutex &m, Condition &c)
{
    std::cout << "thread 1 waiting on condition" << std::endl;
    m.lock();
    c.wait(m);
    std::cout << "thread 1 woke up from c.wait; exiting" << std::endl;
    m.unlock();
}

void
cond_basic_thread2(Mutex &m, Condition &c)
{
    std::cout << "thread 2 locking mutex" << std::endl;
    m.lock();
    std::cout << "thread 2 notifying condition, then yielding "
            "while holding lock" << std::endl;
    c.notify_one();
    Thread::yield();
    std::cout << "thread 2 unlocking mutex, then yielding again" << std::endl;
    m.unlock();
    Thread::yield();
    std::cout << "thread 2 woke up from yield, notifying again" << std::endl;
    m.lock();
    c.notify_one();
    m.unlock();
    std::cout << "thread 2 yielding one last time" << std::endl;
    Thread::yield();
    std::cout << "thread 2 back from final yield" << std::endl;
}

void
cond_basic_test()
{
    // One thread waits on a condition and another thread notifies it.
    
    Mutex m;
    Condition c;

    new Thread([&m, &c] {cond_basic_thread1(m, c);});
    new Thread([&m, &c] {cond_basic_thread2(m, c);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
two_conds_thread1(Mutex &m, Condition &c1, Condition &c2)
{
    std::cout << "thread 1 waiting on condition 1" << std::endl;
    m.lock();
    c1.wait(m);
    std::cout << "thread 1 woke up from c1.wait; waiting again" << std::endl;
    c1.wait(m);
    std::cout << "thread 1 woke up again; exiting" << std::endl;
    m.unlock();
}

void
two_conds_thread2(Mutex &m, Condition &c1, Condition &c2)
{
    std::cout << "thread 2 waiting on condition 2" << std::endl;
    m.lock();
    c2.wait(m);
    std::cout << "thread 2 woke up from wait; exiting" << std::endl;
    m.unlock();
}

void
two_conds_thread3(Mutex &m, Condition &c1, Condition &c2)
{
    std::cout << "thread 3 notifying condition 1, then yielding" << std::endl;
    m.lock();
    c1.notify_one();
    m.unlock();
    Thread::yield();
    std::cout << "thread 3 notify_alling condition 1, then yielding" << std::endl;
    m.lock();
    c1.notify_all();
    m.unlock();
    Thread::yield();
    std::cout << "thread 3 notifying condition 2, then yielding" << std::endl;
    m.lock();
    c2.notify_one();
    m.unlock();
    Thread::yield();
    std::cout << "thread 3 woke up from yield" << std::endl;
}

void
two_conds_test()
{
    // Two threads wait on condition variables and a third thread
    // notifies the conditions.
    
    Mutex m;
    Condition c1, c2;
    
    new Thread([&m, &c1, &c2] {two_conds_thread1(m, c1, c2);});
    new Thread([&m, &c1, &c2] {two_conds_thread2(m, c1, c2);});
    new Thread([&m, &c1, &c2] {two_conds_thread3(m, c1, c2);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
notify_all_thread(Mutex &m, Condition &c, int thread_num)
{
    std::cout << "thread " << thread_num << " waiting on condition" << std::endl;
    m.lock();
    c.wait(m);
    std::cout << "thread " << thread_num << " woke up after wait; exiting"
            << std::endl;
    m.unlock();
}

void
notify_all_control_thread(Mutex &m, Condition &c)
{
    std::cout << "control thread notify_alling condition" << std::endl;
    m.lock();
    c.notify_all();
    m.unlock();
    std::cout << "control thread yielding" << std::endl;
    Thread::yield();
    std::cout << "control thread woke up from yield" << std::endl;
}

void
notify_all_test()
{
    Mutex m;
    Condition c;

    for (int i = 0; i < 5; i++) {
        new Thread([&m, &c, i] {notify_all_thread(m, c, i);});
    }
    new Thread([&m, &c] {notify_all_control_thread(m, c);});
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

void
lock_vs_notify_mutex_thread(Mutex &m, int thread_num)
{
    std::cout << "thread " << thread_num << " waiting on mutex" << std::endl;
    m.lock();
    std::cout << "thread " << thread_num << " woke up after wait; exiting"
            << std::endl;
    m.unlock();
}

void
lock_vs_notify_cv_thread(Mutex &m, Condition &c)
{
    std::cout << "cv thread acquiring mutex" << std::endl;
    m.lock();
    std::cout << "cv thread waiting on condition" << std::endl;
    c.wait(m);
    std::cout << "cv thread woke up after wait; exiting" << std::endl;
    m.unlock();
}

void
lock_vs_notify_control_thread(Mutex &m, Condition &c)
{
    std::cout << "control thread acquiring mutex" << std::endl;
    m.lock();
    std::cout << "control thread yielding" << std::endl;
    Thread::yield();
    std::cout << "control thread notifying cv" << std::endl;
    c.notify_one();
    std::cout << "control thread creating a new mutex thread" << std::endl;
    new Thread([&m] {lock_vs_notify_mutex_thread(m, 3);}); 
    std::cout << "control thread releasing lock and exiting" << std::endl;
    m.unlock();
}

void
lock_vs_notify_test()
{
    // This test tests what happens if there are threads already waiting
    // on the lock when another thread wakes up from a condition variable
    // wait (it must wait on the lock's queue behind the other threads).)
    Mutex m;
    Condition c;

    new Thread([&m, &c] {lock_vs_notify_cv_thread(m, c);});
    new Thread([&m, &c] {lock_vs_notify_control_thread(m, c);});
    for (int i = 0; i < 3; i++) {
        new Thread([&m, i] {lock_vs_notify_mutex_thread(m, i);});
    }
    intr_enable(false);
    Thread::redispatch();
}

//--------------------------------------------------------------------------

int
main(int argc, char **argv)
{
    if (argc == 1) {
        std::cout << "Available tests are:" << std::endl
                << "  enter" << std::endl
                << "  ping_pong" << std::endl
                << "  round_robin" << std::endl
                << "  block" << std::endl
                << "  exit" << std::endl
                << "  exit2" << std::endl
                << "  stack" << std::endl
                << "  preempt" << std::endl
                << "  mutex_basic" << std::endl
                << "  mutex_many_threads" << std::endl
                << "  cond_basic" << std::endl
                << "  two_conds" << std::endl
                << "  notify_all" << std::endl
                << "  lock_vs_notify" << std::endl;
    }
    for (int i = 1; i < argc; i++) {
        // The tests below here are arranged in order from easiest to
        // hardest. The first tests are for Project 2 (thread dispatching).
        if (strcmp(argv[i], "enter") == 0) {
            enter_test();
        } else if (strcmp(argv[i], "ping_pong") == 0) {
            ping_pong_test();
        } else if (strcmp(argv[i], "round_robin") == 0) {
            round_robin_test();
        } else if (strcmp(argv[i], "block") == 0) {
            block_test();
        } else if (strcmp(argv[i], "exit") == 0) {
            exit_test();
        } else if (strcmp(argv[i], "exit2") == 0) {
            exit2_test();
        } else if (strcmp(argv[i], "stack") == 0) {
            stack_test();
        } else if (strcmp(argv[i], "preempt") == 0) {
            preempt_test();

        // Tests below here are for Project 4 (synchronization)
        } else if (strcmp(argv[i], "mutex_basic") == 0) {
            mutex_basic_test();
        } else if (strcmp(argv[i], "mutex_many_threads") == 0) {
            mutex_many_threads_test();
        } else if (strcmp(argv[i], "mutex_mine") == 0) {
            mutex_mine_test();
        } else if (strcmp(argv[i], "cond_basic") == 0) {
            cond_basic_test();
        } else if (strcmp(argv[i], "two_conds") == 0) {
            two_conds_test();
        } else if (strcmp(argv[i], "notify_all") == 0) {
            notify_all_test();
        } else if (strcmp(argv[i], "lock_vs_notify") == 0) {
            lock_vs_notify_test();
        } else {
            std::cout << "No test named '" << argv[i] << "'" << std::endl;
        }
    }
}
