/**
 * File: timeout.cc
 * -----------------
 * Allows the supplied executable to execute for up to a specified
 * number of seconds.  If the process finishes before that
 * time is up, then time-out returns immediately, returning the exit
 * status of the process it timed.  If the executable doesn't finish
 * in the allotted time, then the process is terminated, returning an
 * exit status of 124.
 */

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[]) {
  // Your implementation here!
  pid_t time_command = fork();
  if (time_command == 0) {
    execvp(argv[2], argv + 2);
    printf("error!!");
    return 0;
  }

  pid_t time = fork();
  if (time == 0) {
    sleep(atoi(argv[1]));
    printf("working smooth!\n");
    return 0;
  }

  int status;
  pid_t first = waitpid(-1, &status, 0);
  pid_t second;
  if (first == time_command) {
    second = time;
  }
  else {
    second = time_command;
  }

  kill(second, SIGKILL);
  waitpid(second, NULL, 0);

  if (first == time_command) {
    return WEXITSTATUS(status);
  }
  return 124;

}
