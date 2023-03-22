/**
 * File: fork-utils.h
 * ------------------
 * If #included, this function redefines fork() to include
 * a call to prctl(PR_SET_PDEATHSIG, SIGKILL) in the child before it
 * returns.
 */

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>

static inline pid_t SAFEFORK() {
  pid_t pid = fork();
  if (pid == 0) prctl(PR_SET_PDEATHSIG, SIGKILL);
  return pid;
}

#define fork() (SAFEFORK())
