#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <string.h>

int main(int argc, char** argv)
{
  if (argc <= 1)
  {
    std::cerr << "Usage: ..." << std::endl;
    return 1;
  }

  char* app = argv[1];
  char** args = &argv[1];

  pid_t child = fork();
  if (child == 0)
  {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execv(app, args);
  }
  else
  {
    pid_t wait_outcome = wait(NULL);
    std::cerr << wait_outcome << " " << errno << std::endl;
    long instructions_number = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
    if (instructions_number == -1)
    {
      std::cerr << "error: " << std::string(strerror(errno)) << std::endl;
    }
    std::cerr << "child syscall: " << instructions_number << std::endl;
    ptrace(PTRACE_CONT, child, NULL, NULL);

    wait_outcome = wait(NULL);
    std::cerr << wait_outcome << " " << errno << std::endl;
    wait_outcome = wait(NULL);
    std::cerr << wait_outcome << " " << errno << std::endl;
  }
  return 0;
}
