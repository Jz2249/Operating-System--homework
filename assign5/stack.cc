
#include <cassert>
#include <cstdlib>
#include <unistd.h>

#include "stack.hh"
#include "thread.hh"

int Stack::num_stacks_ = 0;

// Determines how many registers must be saved on the stack, not
// including return address.
#if __x86_64
const int saved_regs = 6;
#elif __i386
const int saved_regs = 4;
#else
#error "unsupported architecture"
#endif

#include <stdio.h>
void print_if(const char *p) {
    if (p != nullptr) {
        puts(p);
    }
}

// If a thread ever returns off the top of its stack, it will return
// to this method, which aborts with an error message.
[[noreturn]] static void
stack_underflow()
{
    // Be careful: when this method is invoked, we've run off the top
    // of the stack, so there is no call frame!
    static const char msg[] = "returned off the top of a thread stack\n";
    if (write(2, msg, sizeof(msg)-1)) {
        /* Ignore write errors. */
    }
    std::abort();
}

Stack::Stack(void (*start)(Thread *), Thread *t)
    :  sp_(nullptr),
       stack_()
{
    // Set the top of the stack to look like this:
    //       -----------------------------------
    //       |        stack_underflow          |
    //       -----------------------------------
    //       |             start               |
    //       -----------------------------------
    //       |                                 |
    //       |     space for saved_regs        |
    //       |     registers (dummy values)    |
    // sp -> |                                 |
    //       -----------------------------------
    // In other words, the stack will look as if switch_to was
    // invoked by code immediately preceding the first instruction
    // of the start function; it will also appear as if that
    // code was invoked by code immediately preceding the
    // first instruction of stack_underflow. Thus, when switch_to
    // returns back into this stack, it will go to the beginning
    // of the start function. If that function should return (which it
    // shouldn't) then stack_underflow will be invoked.
    
    uintptr_t* top = reinterpret_cast<uintptr_t *>(
            stack_ + sizeof(stack_));
    top[-1] = reinterpret_cast<uintptr_t>(stack_underflow);
    top[-2] = reinterpret_cast<uintptr_t>(start);
    sp_ = top - 2 - saved_regs;

    // Nick T.: this is a total hack to make it such that start
    // is passed t as its parameter.  The stack_switch function
    // was updated to also move %r15 (this last register popped)
    // into %rdi (cheezy/hacky, I know... but it removes the need
    // for the thread wrapper to have to call Thread::current
    // since the thread being run is passed in directly).
    sp_[0] = reinterpret_cast<uintptr_t>(t);
    
    num_stacks_++;
}

Stack::~Stack()
{
    num_stacks_--;
}

// The stack_switch function follows below; it is implemented in assembly
// in order to get access to the hardware registers. There are different
// implementations for 32-bit and 64-bit modes.

#if __x86_64

/*
  The x86_64 architecture has the following calling convention:

  [integer class] argument passing:
  - First six arguments: go in %rdi, %rsi, %rdx, %rcx, %r8, %r9
  - Remaining arguments pushed to stack (last arg pushed first)

  return value: %rax

  callee-saved registers: %rbp, %rbx, %r12, %r13, %r14, %r15

  stack pointer: %rsp (also callee-saved)
*/

asm(R"(.text
	.global stack_switch
stack_switch:
	pushq %rbp
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

    test %rdi,%rdi
    je restore
	movq %rsp,(%rdi)	# *prev_sp = %rsp
restore:
	movq (%rsi),%rsp	# %rsp = *next_sp

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	popq %rbp
    movq %r15,%rdi  # NT hack to be able to specify %rdi on thread start

	ret
)");

#elif __i386

// TODO: NT - the hack for x86-64 for %rdi wasn't carried over to i386

/*
  The i386 architecture has the following calling convention:

  argument passing: all pushed to stack (in reverse order)

  return value: %eax

  callee-saved registers: %ebp, %edi, %esi, %ebx

  stack pointer: %esp (callee-saved)
*/

asm(R"(.text
	.global stack_switch
stack_switch:
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx

	movl 20(%esp), %eax	# %eax = prev_sp
	movl 24(%esp), %edx	# %edx = next_sp
    test %eax,%eax
    je restore
	movl %esp,(%eax)	# *prev_sp = %esp
restore:
	movl (%edx),%esp	# %esp = *next_sp

	popl %ebx
	popl %esi
	popl %edi
	popl %ebp

	ret
)");

#else
#error "unsupported architecture"
#endif