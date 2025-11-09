#include <sys/prctl.h>

#include <iostream>
#include <array>
#include <string>
#include <cstring>
#include <thread>

#include "ipc.h"

int main(int argc, char *argv[])
{
    if (ipcInit("transmitter") != 0)
    {
        std::cout << "Failed to initialize ipc\n";
        return -1;
    }
    std::cout << "Create IPC for transmitter\n";

    std::array<char, 128> buffer;
    std::string result;

    while (true)
    {
        auto pipe = popen("lspci", "r");

        if (!pipe)
        {
            perror("popen");
            return EXIT_FAILURE;
        }

        // feof tests the end-of-file indicator for the given stream.
        while (!feof(pipe))
        {
            if (fgets(buffer.data(), 128, pipe) != nullptr)
            {
                result += buffer.data();
            }
        }

        auto rc = pclose(pipe);

        if (rc == EXIT_FAILURE) 
        {
            perror("pclose");
            break;
        }

        LSPCIOutputMsg msg;
        msg.msg_type = MsgType::LSPCI_OUTPUT;
        memcpy(msg.lspci_output, result.c_str(), sizeof(msg.lspci_output));
        ipcSend("receiver", (char *)&msg, sizeof(msg));

        result.clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ipcDestroy();
    return 0;
}
