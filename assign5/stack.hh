
#pragma once

#include <cstdint>

class Thread;

// This class represents a stack: it has space for the actual stack, plus
// room to save the current stack pointer when the stack is not in use.
class Stack {
public:
    /* Constructor: initializes the stack so that the first time you
     * switch to it, the function `start` will run from its beginning, 
     * and will be passed the specified thread as its parameter.  start
     * should take one parameter, which is a pointer to the running thread,
     * and should never return.
     */
    Stack(void(*start)(Thread *), Thread *t);

    
    ~Stack();
    
    // Total number of Stack objects currently in existence (used for testing).
    static int num_stacks_;
    
private:
    // Saved stack pointer from the last time stack_switch was invoked
    // on this Stack.
    uintptr_t *sp_;
    
    // Space for the actual stack.
    char stack_[8192];
};

/* This function performs a context switch.  It saves registers on the 
 * current stack, and saves the current stack pointer in current 
 * (only if current is not null). Then loads the stack pointer from next 
 * and restores registers from that stack. When this function returns, 
 * it will return from the last stack_switch call made on next.
 */
extern "C" void stack_switch(Stack *current, Stack *next);
