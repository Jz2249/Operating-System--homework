/**
 * File: stsh.cc
 * -------------
 * Defines the entry point of the stsh executable.
 */

#include <iostream>
#include <fcntl.h>
#include <unistd.h>   // for fork
#include <sys/wait.h> // for waitpid
#include "stsh-parser/stsh-parse.h"
#include "stsh-parser/stsh-readline.h"
#include "stsh-exception.h"
#include "fork-utils.h" // this needs to be the last #include in the list

using namespace std;

void output_redirection(const pipeline& p);
void input_redirection(const pipeline& p);
void middle_process(int read, int write, command cmd);
void execvp_or_throw(command cmd);
void wait_child(int len, pid_t pidOrZero[]);


/**
 * Create new process(es) for the provided pipeline. Spawns child processes with
 * input/output redirected to the appropriate pipes and/or files.
 */
void runPipeline(const pipeline& p) {
  // Remove the placeholder code below and add your implementation.
  int len = p.commands.size();
  pid_t pidOrZero[len];
  int pipe_len = len - 1;
  int fds[pipe_len][2];

  if (len > 1) pipe2(fds[0], O_CLOEXEC);
  pidOrZero[0] = fork();

  command cmd = p.commands[0];
  if (pidOrZero[0] == 0) { // the first command
    cmd = p.commands[0];
    if (!p.input.empty()) {
      input_redirection(p);
    }
    if (len == 1){
      if (!p.output.empty()) { // if only one command, it is the first one and the last one
        output_redirection(p);
      }
    }
    else {
      dup2(fds[0][1], STDOUT_FILENO);
    }
    execvp_or_throw(cmd);
  }

  // middle part between the first and the last command
  for (int i = 1; i < len; i++) {
    cmd = p.commands[i];
    if (i != len - 1) pipe2(fds[i], O_CLOEXEC); // len-2 pipes total
    pidOrZero[i] = fork();
    if (i == len - 1) break;
    if (pidOrZero[i] == 0) { // have at least two commands and not the first or last child
      middle_process(fds[i - 1][0], fds[i][1], cmd);
    }
  }

  // last child (special case: standard input redirect to previous standard output)
  if (pidOrZero[len - 1] == 0) { 
    dup2(fds[pipe_len - 1][0], STDIN_FILENO);
    if (!p.output.empty()) {
      output_redirection(p);
    }
    execvp_or_throw(cmd);
  }

  
  for (int i = 0; i < len - 1; i++) {
    close(fds[i][0]);
    close(fds[i][1]);
  }
  wait_child(len, pidOrZero);
}


void wait_child(int len, pid_t pidOrZero[]) {
  int is_segment = 0;
  for (int i = 0; i < len; i++) {
    int status;
    waitpid(pidOrZero[i], &status, 0);
    if (WIFSIGNALED(status)) {
      if (WTERMSIG(status) == SIGSEGV) {
        is_segment = 1;
      }
    }     
  }
  if (is_segment) cerr << "Segmentation fault" << endl;   
}
void execvp_or_throw(command cmd) {
  execvp(cmd.argv[0], cmd.argv);
  throw STSHException(string(cmd.argv[0]) + ": Command not found.");
}
void middle_process(int read, int write, command cmd) {
  dup2(read, STDIN_FILENO);
  dup2(write, STDOUT_FILENO);
  execvp_or_throw(cmd);
}
void input_redirection(const pipeline& p) {
  int fd = open(p.input.c_str(), O_RDONLY | O_EXCL, 0644);
  if (fd == -1) throw STSHException(string("Could not open ") + '"' + p.input + '"' + ".");
  dup2(fd, STDIN_FILENO);
  close(fd);
}

void output_redirection(const pipeline& p) {
  int fd = open(p.output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) throw STSHException(string("Could not open ") + '"' + p.output + '"' + ".");
  dup2(fd, STDOUT_FILENO);
  close(fd);
}


/**
 * Define the entry point for a process running stsh.
 */
int main(int argc, char *argv[]) {
  pid_t stshpid = getpid();
  rlinit(argc, argv);
  while (true) {
    string line;
    if (!readline(line) || line == "quit" || line == "exit") break;
    if (line.empty()) continue;
    try {
      pipeline p(line);
      runPipeline(p);
    } catch (const STSHException& e) {
      cerr << e.what() << endl;
      if (getpid() != stshpid) exit(0); // if exception is thrown from child process, kill it
    }
  }
  return 0;
}
