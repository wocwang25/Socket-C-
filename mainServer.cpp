#include "Server.h"
#include "Server.cpp"
int main()
{
    Socket server;
    Connection cnt;
    cnt.create_connection(server);
    cnt.handle_connection(server);
    return 0;
}