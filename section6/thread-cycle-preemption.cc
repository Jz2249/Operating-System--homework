/* thread-cycle.cc
 * This program shows an example of how we can make multiple threads, each
 * storing the "next" thread they yield to, and running them in a cycle such that
 * each thread is switched to twice.  It uses the context_switch function, which
 * does a stack switch to another thread.
 */

#include <iostream>
#include "timer.hh"

using namespace std;

// The number of registers in x86 we must save on the stack during a context switch
static const int kNumRegistersToSave = 6;

/* We must declare that there is a type Thread so that we can declare current,
 * since current is also used in Thread (so it must come first).
 */
struct Thread;

// The currently-running thread
static Thread *current = nullptr;


/* A thread is a stack plus the current top of the stack,
 * plus a name plus a pointer to the thread it should yield to.
 */
typedef struct Thread {

    Thread(void (*func)(), const string& thread_name, Thread *next_thread) {
        name = thread_name;
        next = next_thread;

        /* This sets up a thread so that the first time it is switched to,
         * it "resumes" by starting to run the specified func.
         */

        void *stack_top = stack + sizeof(stack);

        // Make it look like this thread was about to start func,
        // and then context switched.  In other words, once
        // the saved registers are popped, ret should take us to
        // the start of func.
        *(void **)((char *)stack_top - sizeof(void *)) = (void *)func;

        // Move the stack pointer downwards by 6 registers to make it appear like
        // we pushed 6 registers on when we previously context switched
        saved_rsp = (char *)stack_top - sizeof(void *) - kNumRegistersToSave * sizeof(long);
    }

    Thread(const string& thread_name) {
        name = thread_name;
    }

    char stack[8192];
    char *saved_rsp;
    string name;
    Thread *next;
} Thread;

/* This function context_switch is written in assembly!  It switches from running 'current'
 * to running 'next'.  To do this, tt first pushes 'current's registers onto the stack
 * to save them, then updates 'current' with the new stack pointer.  Then it loads 'next's
 * stack pointer and from then on we are operating on 'next's stack, not current's!  So we
 * proceed to pop off all the registers we left on 'next's' stack from before, and then
 * return, and this will return back to the function that 'next' had been executing. 
 * 'current' will be resumed when another thread calls context_switch to switch back to
 * its stack.
 */
extern "C" void context_switch(Thread& current, Thread& next);

asm(R"(.text
    .global context_switch
context_switch:
    pushq %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp,0x2000(%rdi)      # saved current's stack pointer
    movq 0x2000(%rsi),%rsp      # load next's stack pointer
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret     # return back to the function **this stack** was running
)");

// Yield to the next thread and let it run
void yield() {
    Thread *prev = current;
    current = current->next;
    cout << "Going from thread " << prev->name << " to thread " << current->name << endl;
    context_switch(*prev, *current);
    cout << "Back in thread " << prev->name << endl;
}

// Function run by all non-main threads
void thread_run() {
    intr_enable(true);
    while (true) {
        cout << "Hello, I am thread " << current->name << endl;
    }
}

void timer_interrupt_handler() {
    yield();
}

int main(int argc, char *argv[]) {
    // Make a Thread variable to represent this main thread, which is currently running
    Thread main_thread("main");
    current = &main_thread;

    // Make two more threads, and link them such that main yields to two,
    // two yields to one, and one yields to main
    Thread one(thread_run, "one", &main_thread);
    Thread two(thread_run, "two", &one);
    main_thread.next = &two;

    // Fire the timer every 500,000 microseconds to context switch
    timer_init(500000, timer_interrupt_handler);

    while (true) {
        cout << "Hello, I am the main thread" << endl;
    }
}
