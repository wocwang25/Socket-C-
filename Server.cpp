#include "Server.h"

void Connection::set_nonblocking(int &fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("F_GETFL failed!");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("F_SETFL failed!");
        exit(EXIT_FAILURE);
    }
}
void Connection::set_EdgeTriggered(int &fd)
{
    this->ev.events = EPOLLIN | EPOLLET;
    this->ev.data.fd = fd;
    epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &this->ev);
}

void Connection::set_LevelTriggered(int &fd)
{
    this->ev.events = EPOLLIN;
    this->ev.data.fd = fd;
    epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &this->ev);
}

bool Connection::check_user_connected(const int &fd)
{
    if (this->transfer_fds.empty())
        return false;

    for (int i = 0; i < this->transfer_fds.size(); i++)
    {
        if (this->transfer_fds[i] == fd)
            return true;
    }
    return false;
}

// data: "msg\nfd"
// args for recv: fd, buffer, size of buffer, flag 0 -> using while loop to take all the data
void Connection::handle_received_data(const int &fd, DataParser &dtp)
{
    int n = 0;
    while (true)
    {
        n = recv(fd, dtp.buffer + dtp.size_buffer, sizeof(dtp.buffer), 0);
        if (n <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            break;
        }
        dtp.size_buffer += n;
    }
    dtp.buffer[dtp.size_buffer] = '\0';
    // printf("data buffer: %s with len %d\n", dtp.buffer, dtp.size_buffer);
    if (dtp.size_buffer > 0)
    {
        dtp.parser();
    }
    if (memcmp(dtp.fd, "", 2) != 0)
    {
        int fd = atoi(dtp.fd);
        if (check_user_connected(fd) == false)
        {
            printf("User fd %d isn't connected!\n", fd);
            exit(EXIT_FAILURE);
        }
    }
}

// send with full buffer in dtp, fd and msg in dtp is just for server to take the receiver fd
void Connection::handle_send_data(const DataParser &dtp)
{
    int receiver_fd = atoi(dtp.fd);
    int n = 0, sent = 0;

    // printf("data parser: {%s - %s}\n", dtp.msg, dtp.fd);
    while (true)
    {
        n = send(receiver_fd, dtp.buffer + n, dtp.size_buffer, 0);
        printf("n server sent: %d\n", n);
        sent += n;
        if (n <= 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            break;
        }
        if (sent == dtp.size_buffer)
        {
            break;
        }
    }
}

void Connection::create_connection(Socket &server)
{
    server.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server.fd <= 0)
    {
        perror("Create socket failed!");
        exit(EXIT_FAILURE);
    }
    server.addr.sin_family = AF_INET;
    server.addr.sin_port = htons(PORT);
    server.addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (void *)&opt, sizeof(opt)))
    {
        perror("Setsockopt failed!");
        exit(EXIT_FAILURE);
    }
    if (bind(server.fd, (struct sockaddr *)&server.addr, sizeof(server.addr)) < 0)
    {
        perror("Bind failed!");
        exit(EXIT_FAILURE);
    }

    set_nonblocking(server.fd);
    if (listen(server.fd, BACK_LOG) < 0)
    {
        perror("Open listen socket failed!");
        exit(EXIT_FAILURE);
    }
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server.addr.sin_addr, ip, INET_ADDRSTRLEN);
    printf("Open listen fd %d at address %s:%d\n", server.fd, ip, ntohs(server.addr.sin_port));
}

void Connection::handle_connection(Socket &server)
{
    set_EdgeTriggered(server.fd);
    printf("Waiting for event~~\n");
    while (true)
    {
        this->nfds = epoll_wait(this->epfd, this->events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (this->events[i].data.fd == server.fd)
            {
                printf("New connection~\n");
                int addr_len = sizeof(server.addr);
                int fd = 0;

                fd = accept(server.fd, (struct sockaddr *)&server.addr, (socklen_t *)&addr_len);
                if (fd < 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;
                    break;
                }

                char cli_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &server.addr.sin_addr, cli_ip, INET_ADDRSTRLEN);
                printf("Client %s:%d connect to server with fd %d\n", cli_ip, ntohs(server.addr.sin_port), fd);

                this->transfer_fds.push_back(fd);
                set_nonblocking(fd);
                set_EdgeTriggered(fd);
            }
            else
            {
                printf("New message~\n");
                DataParser dtp;
                handle_received_data(this->events[i].data.fd, dtp);
                handle_send_data(dtp);
            }
        }
    }
}
/*
parser "helo\n5"
parser "helo\n6"
msg: helo\n - fd: 5
msg: helo\n - fd: 6
check trong transfer_fds có fd chưa -> return User isn't connected, try later

*/
