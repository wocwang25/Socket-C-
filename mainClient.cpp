#include "Client.h"
#include "Client.cpp"

int main()
{
    ClientConnection clc;
    Connection cnt;
    clc.create_connection();
    clc.input_receiver();
    clc.handle_connection();
    return 0;
}