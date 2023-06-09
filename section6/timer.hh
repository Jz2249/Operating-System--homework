
#pragma once

#include <functional>
#include <cstdint>

// Invoke handler (with interrupts disabled) every usec microseconds.
// If usec is 0 or handler is nullptr, removes the timer interrupt.
void timer_init(std::uint64_t usec, std::function<void()> handler);

// Returns true if interrupts are enabled
bool intr_enabled();

// If the argument is false, defers all interrupts.  If the argument
// is true, enables interrupts and immediately dispatches any deferred
// interrupts.
void intr_enable(bool on);

// Rather than call intr_enable directly, put an object of this
// type on the stack to disable interrupts.  That way interrupts will
// be reenabled automatically when the object is destroyed. These
// objects nest properly (e.g. interrupts will be off as long as there
// is at least one active IntrGuard object on the current stack).
class IntrGuard {
    const bool old_state_;
public:
    IntrGuard() : old_state_(intr_enabled()) { intr_enable(false); }
    ~IntrGuard() { intr_enable(old_state_); }
};
