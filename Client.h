#ifndef CLIENT_H
#define CLIENT_H

#include "Server.h"
#include "Server.cpp"

class ClientConnection
{
public:
    Socket client;
    char receiver[10];

public:
    ClientConnection() {};
    void create_connection();
    // dtp for handle msg,
    void handle_connection();
    void input_receiver();
    void input_msg(DataParser &dtp);
    void receive_msg();
    void send_msg(DataParser &dtp);
};

#endif
