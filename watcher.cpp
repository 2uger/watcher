#include <iostream>
#include <thread>
#include <string>
#include <array>
#include <map>

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// List of all binaries you want to run
std::array<std::string, 2> binaries_to_run = {
    "transmitter",
    "receiver"
};

struct ChildInfo
{
    std::string bin_path;
};

static std::map<pid_t, ChildInfo> child_processes;

static pid_t runChildProcess(std::string &app_path)
{
    pid_t pid;
    pid = fork();

    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        execv(app_path.c_str(), NULL);
        perror("execv");
        return -1;
    }
    return pid;
}

int main(int argc, char **argv)
{
    std::cout << "Watcher started\n";

    char cwd_buffer[512];
    for (auto &bin_path : binaries_to_run)
    {
        getcwd(cwd_buffer, 512);
        std::string cwd_string(cwd_buffer);
        cwd_string = cwd_string + "/" + bin_path;
        std::cout << cwd_string << "\n";

        pid_t child_pid = runChildProcess(cwd_string);
        if (child_pid == -1)
        {
            std::cerr << "Failed to run child process, kill already created and exit\n";
            for (auto &[pid, child_info] : child_processes)
            {
                kill(pid, SIGKILL);
            }
            return EXIT_FAILURE;
        }
        child_processes[child_pid] = ChildInfo{bin_path};
    }

    while (true)
    {
        for (auto &[pid, child_info] : child_processes)
        { 
            int wstatus;
            int ret = waitpid(pid, &wstatus, WNOHANG);

            if (ret != 0)
            {
                std::cout << "Wstatus: " << wstatus << "\n";
                std::cout << "Child process finished for pid: " << pid << ", bin path: " << child_info.bin_path << ", rerun it\n";

                child_processes[runChildProcess(child_info.bin_path)] = ChildInfo{child_info.bin_path};
                child_processes.erase(pid);
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
