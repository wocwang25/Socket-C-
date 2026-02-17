#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <vector>
#define MAX_CHAR 1024

class DataParser
{
public:
    char buffer[MAX_CHAR];
    char msg[MAX_CHAR];
    char fd[10];
    char size_buffer;

public:
    DataParser()
    {
        memset(this->buffer, 0, sizeof(this->buffer));
        memset(this->msg, 0, sizeof(this->msg));
        memset(this->fd, 0, sizeof(this->fd));
        this->size_buffer = 0;
    }
    void parser();
};

#endif