#include <unistd.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <string.h>

#include <iostream>
#include <unordered_map>
#include <chrono>
#include <thread>

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
      long syscall = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
      if (syscall == -1)
      {
        std::cerr << "error: " << std::string(strerror(errno)) << std::endl;
        break;
      }
      auto syscall_name = syscall_names.at(syscall);
      if (syscall_name == "sendto" or syscall_name == "sendmsg" or syscall_name == "sendmmsg")
      {
        auto latency = std::chrono::milliseconds{5000};
        std::cerr << "adding latency (" << latency.count() << " ms) to child syscall: "
                  << syscall << " (" << syscall_name << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds{5000});
      }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }
  }
  return 0;
}
