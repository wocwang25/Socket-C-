#include "Client.h"

void ClientConnection::create_connection()
{
    this->client.fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->client.fd < 0)
    {
        perror("Create socket failed!");
        exit(EXIT_FAILURE);
    }
    this->client.addr.sin_family = AF_INET;
    this->client.addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &this->client.addr.sin_addr) <= 0)
    {
        perror("Invalid Address!");
        exit(EXIT_FAILURE);
    }

    if (connect(this->client.fd, (struct sockaddr *)&this->client.addr, (socklen_t)sizeof(this->client.addr)) < 0)
    {
        perror("Connect to server failed!");
        exit(EXIT_FAILURE);
    }
}

void ClientConnection::handle_connection()
{
    Connection cnt;
    // cho nay la set nonblocking cho input
    int fd = STDIN_FILENO;
    cnt.set_nonblocking(fd);
    // set cho cai fd cua ben phia client
    cnt.set_nonblocking(this->client.fd);

    cnt.set_EdgeTriggered(this->client.fd);
    cnt.set_LevelTriggered(fd);

    while (true)
    {
        DataParser dtp;
        printf("\nmsg: ");
        fflush(stdout);
        cnt.nfds = epoll_wait(cnt.epfd, cnt.events, MAX_EVENTS, -1);
        for (int i = 0; i < cnt.nfds; i++)
        {
            if (cnt.events[i].data.fd == STDIN_FILENO)
            {
                send_msg(dtp);
            }
            else if (cnt.events[i].data.fd == this->client.fd)
            {
                receive_msg();
            }
            else
            {
                perror("Strange Situation!");
                break;
            }
        }
    }
}

void ClientConnection::input_receiver()
{
    printf("Input receiver transfer fd (view on server): ");
    fgets(this->receiver, sizeof(this->receiver), stdin);
}

void ClientConnection::input_msg(DataParser &dtp)
{
    while (true)
    {
        int n = read(STDIN_FILENO, dtp.msg + dtp.size_buffer, MAX_CHAR - dtp.size_buffer);
        if (n <= 0)
        {
            if (n == 0)
                break;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            else
            {
                perror("Read buffer from STDIN_FILENO failed!");
                exit(EXIT_FAILURE);
            }
        }
        dtp.size_buffer += n;
    }

    if (memcmp(dtp.msg, "bye", 3) == 0)
    {
        printf("End connection\n");
        exit(EXIT_SUCCESS);
    }
    memcpy(dtp.msg + dtp.size_buffer, this->receiver, strlen(this->receiver));
    dtp.size_buffer += strlen(this->receiver);
    dtp.msg[dtp.size_buffer] = '\0';
}

void ClientConnection::receive_msg()
{
    DataParser dtp;
    char temp[MAX_CHAR];
    while (true)
    {
        int n = recv(this->client.fd, dtp.buffer + dtp.size_buffer, sizeof(dtp.buffer), 0);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("Receive failed!");
            exit(EXIT_FAILURE);
        }
        if (n == 0)
        {
            printf("Server close connection!\n");
            exit(EXIT_SUCCESS);
        }
        dtp.size_buffer += n;
    }
    if (dtp.size_buffer > 0)
    {
        dtp.buffer[dtp.size_buffer] = '\0';
        dtp.parser();
    }
    printf("(recv) %s", dtp.msg);
}

void ClientConnection::send_msg(DataParser &dtp)
{
    input_msg(dtp);
    int n = 0, sent = 0;
    while (true)
    {
        n = send(this->client.fd, dtp.msg, strlen(dtp.msg), 0);
        // printf("n client sent: %d\n", n);
        sent += n;
        if (n <= 0)
        {
            if (n == 0)
                break;
            perror("Send failed!");
            exit(EXIT_FAILURE);
        }
        if (sent == strlen(dtp.msg))
            break;
    }
}
