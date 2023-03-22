
#include <cassert>
#include <iostream>
#include <unistd.h>

#include "stack.hh"
#include "thread.hh"
#include "timer.hh"

using namespace std;

/* Static variables must be initialized individually
 * by specifying its type again, followed by Thread::NAME (where NAME
 * is its name) and, if applicable, what we want to initialize it to.
 */
queue<Thread*> Thread::ready_;
Thread* Thread::curr_; // current thread in run
bool Thread::has_running_thread = false;
Stack* Thread::prev_stack;

void
Thread::thread_start(Thread *t)
{
    // You have to implement this
    intr_enable(true);
    t -> func();
    Thread :: exit();
}

Thread::Thread(function<void()> main) : func(main)
{
    // You have to implement this
    IntrGuard guard;
    this -> stack = new Stack(thread_start, this);
    this -> schedule();
}

Thread *
Thread::current()
{
    // Replace the code below with your implementation.

    IntrGuard guard;
    if (!has_running_thread) return nullptr;
    return curr_;
}

void
Thread::schedule()
{
    IntrGuard guard;
    ready_.push(this);

}

void
Thread::redispatch()
{
    assert(!intr_enabled());
    // You have to implement this
    if (ready_.size() == 0) {
        std :: exit(0);
    }
    Stack *current_stack;
    Thread *curr = current();
    Thread *next = ready_.front();
    if (!has_running_thread)  current_stack = nullptr;
    else current_stack = curr -> stack;
    Stack *next_stack = next -> stack;
    curr_ = next;
    has_running_thread = true;
    ready_.pop();
    stack_switch(current_stack, next_stack);
}   

void
Thread::yield()
{
    // You have to implement this
    IntrGuard guard;
    Thread *current_thread = current();
    current_thread -> schedule();
    redispatch();

}

void
Thread::exit()
{
    // You have to implement this
    IntrGuard guard;
    if (prev_stack != nullptr) delete prev_stack;
    if (curr_ != nullptr) {
        prev_stack = curr_ -> stack; 
        delete curr_;
    }
    has_running_thread = false;
    redispatch();
    // Your code above should clean up the thread such that control never
    // reaches here (this method should never return).
    cerr << "Thread::exit() tried to return" << endl;
    abort();

}

void
Thread::preempt_init(uint64_t usec)
{
    // You have to implement this
    timer_init(usec, Thread :: yield);
}
