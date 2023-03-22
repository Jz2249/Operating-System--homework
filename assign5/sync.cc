
#include <iostream>
#include "sync.hh"
#include "timer.hh"

using namespace std;




Mutex::Mutex()
{
    // Fill in any code (if any) that is needed here.
    IntrGuard guard;

    is_lock = false;

}

void
Mutex::lock()
{
    if (mine()) {
        cerr << "locking mutex already locked by this thread" << endl;
        abort();
    }

    // You need to implement the rest of this function
    IntrGuard guard;
    Thread *curr = Thread::current();
    if (!this -> is_lock) {
        is_lock = true;
        lock_owner = curr;
    }
    else {
        blockQueue.push(curr);
        Thread::redispatch();
    }
}

void
Mutex::unlock()
{
    if (!mine()) {
        cerr << "unlocking mutex not held by this thread" << endl;
        abort();
    }

    // You need to implement the rest of this function
    IntrGuard guard;
    if (blockQueue.size() == 0) {
        this -> is_lock = false;
        this -> lock_owner = nullptr;
    }
    else {
        Thread *curr = blockQueue.front();
        this -> lock_owner = curr;
        blockQueue.pop();
        curr -> schedule();
    }
}

bool
Mutex::mine()
{
    // Replace the code below with your implementation.
    IntrGuard guard;
    if (this -> lock_owner == Thread::current()) return true;
    return false;
}

Condition::Condition()
{
    // Fill in any code (if any) that is needed here
}

void
Condition::wait(Mutex &m)
{
    if (!m.mine()) {
        cerr << "Condition::wait must be called with mutex locked"
                << endl;
        abort();
    }

    // You need to implement the rest of this function
    IntrGuard guard;

    Thread *curr = Thread::current();
    blockQueue_.push(curr); 
    m.unlock();
    Thread::redispatch();
    m.lock();
   
}

void
Condition::notify_one()
{
    // You need to implement this function
    IntrGuard guard;
    if (!blockQueue_.empty()) {
        Thread* wakeup_thread = blockQueue_.front();
        blockQueue_.pop();
        wakeup_thread -> schedule();
    }

}

void
Condition::notify_all()
{
    // You need to implement this function
    IntrGuard guard;
    while (!blockQueue_.empty()) {
        Thread* wakeup_thread = blockQueue_.front();
        blockQueue_.pop();
        wakeup_thread -> schedule();
    }
}
