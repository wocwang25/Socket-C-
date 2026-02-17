#ifndef SERVER_H
#define SERVER_H

#include "Utils.h"
#include "Utils.cpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_EVENTS 50
#define BACK_LOG 10
#define IP "127.0.0.1"
#define PORT 8080

class Socket
{
public:
    int fd;
    struct sockaddr_in addr;
    Socket() {};
    ~Socket()
    {
        close(this->fd);
    }
};

class Connection
{
public:
    std::vector<int> transfer_fds;
    int epfd;
    int nfds;
    struct epoll_event ev, events[MAX_EVENTS];

public:
    void set_nonblocking(int &fd);
    void set_EdgeTriggered(int &fd);
    void set_LevelTriggered(int &fd);
    bool check_user_connected(const int &fd);
    void handle_received_data(const int &fd, DataParser &dtp);
    void handle_send_data(const DataParser &dtp);
    void create_connection(Socket &server);
    void handle_connection(Socket &server);

    Connection()
    {
        this->epfd = epoll_create1(0);
    }
    ~Connection()
    {
        for (int i = 0; i < this->transfer_fds.size(); i++)
        {
            if (this->transfer_fds[i])
            {
                close(this->transfer_fds[i]);
            }
        }
        close(this->epfd);
    }
};

#endif