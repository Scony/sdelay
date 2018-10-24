#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <string.h>

#include <iostream>
#include <unordered_map>

extern std::unordered_map<unsigned, std::string> syscall_names;

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
    int exit_status;
    while (true)
    {
      wait(&exit_status);
      if (WIFEXITED(exit_status))
      {
        std::cerr << "child exited: " << exit_status << std::endl;
        break;
      }
      long instructions_number = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
      if (instructions_number == -1)
      {
        std::cerr << "error: " << std::string(strerror(errno)) << std::endl;
        break;
      }
      std::cerr << "child syscall: " << instructions_number
                << " (" << syscall_names.at(instructions_number)
                << ")" << std::endl;
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }
  }
  return 0;
}
