#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"

int server_socket_fd;

const std::string kCommonSocketPath = "/tmp";

int ipcInit(std::string name)
{
    //
    // Initialize server for communication
    //
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_fd == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    std::string socket_path = kCommonSocketPath + "/" + name;
    strncpy(server_addr.sun_path, socket_path.c_str(), socket_path.size());

    unlink(socket_path.c_str());

    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        return -1;
    }
    
    if (listen(server_socket_fd, 10) == -1)
    {
        perror("listen");
        return -1;
    }

    return 0;
}

int ipcDestroy()
{
    if (server_socket_fd)
        close(server_socket_fd);

    return 0;
}

int ipcSend(std::string dst_name, char *msg, size_t sz)
{
    //
    // Initialize client for communication
    //
    int client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket_fd == -1)
    {
        perror("socket");
        close(server_socket_fd);
        return -1;
    }

    struct sockaddr_un dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_un));
    dst_addr.sun_family = AF_UNIX;
    std::string peer_socket_path = kCommonSocketPath + "/" + dst_name;
    strncpy(dst_addr.sun_path, peer_socket_path.c_str(), peer_socket_path.size());

    char *msg_ptr = msg;
    if (client_socket_fd && connect(client_socket_fd, (struct sockaddr *)&dst_addr, sizeof(struct sockaddr_un)) != -1)
    {
        size_t to_send_count = sz;
        while (to_send_count > 0)
        {
            size_t sended = write(client_socket_fd, msg_ptr, to_send_count);
            if (sended == -1)
            {
                close(client_socket_fd);
                return -1;
            }
            to_send_count -= sended;
            msg_ptr += sended;
        }
        close(client_socket_fd);
        return 0;
    }
    close(client_socket_fd);
    return -1;
}

int ipcRecv(char *msg, size_t sz)
{
    struct sockaddr_un peer_addr;
    socklen_t peer_addr_size = sizeof(peer_addr);
    int peer_socket_fd = accept(server_socket_fd, (struct sockaddr *)&peer_addr, &peer_addr_size);
    if (peer_socket_fd == -1)
    {
        perror("accept");
        return -1;
    }

    size_t to_recv_sz = sz;
    char *msg_ptr = msg;
    while (to_recv_sz > 0)
    {
        size_t readed = read(peer_socket_fd, msg_ptr, sz);
        if (readed == -1)
        {
            close(peer_socket_fd);
            return -1;
        }
        to_recv_sz -= readed;
        msg_ptr += readed;
    }
    close(peer_socket_fd);
    return 0;
}

