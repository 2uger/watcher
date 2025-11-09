#include <sys/prctl.h>

#include <iostream>

#include "ipc.h"

int main()
{
    if (ipcInit("receiver") != 0)
    {
        std::cout << "Failed to initialize ipc\n";
        return -1;
    }
    std::cout << "Create IPC for transmitter\n";

    while (true)
    {
        LSPCIOutputMsg msg;
        if (ipcRecv((char *)&msg, sizeof(msg)) == 0)
        {
            std::cout << "Receive lspci output from transmitter:\n" << msg.lspci_output << "\n";
        }
    }

    return 0;
}
