#include <string>
#include <cstdint>

enum MsgType : uint8_t
{
    LSPCI_OUTPUT = 0x1
};

struct LSPCIOutputMsg
{
    MsgType msg_type;
    char lspci_output[4096];
} __attribute__((packed));

int ipcInit(std::string name);
int ipcDestroy();

int ipcSend(std::string dst_name, char *msg, size_t sz);
int ipcRecv(char *msg, size_t sz);

