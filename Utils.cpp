#include "Utils.h"

void DataParser::parser()
{
    int pos = -1;
    for (int i = 0; i < this->size_buffer; i++)
    {
        if (buffer[i] == '\n')
        {
            pos = i;
            break;
        }
    }
    if (pos == -1)
    {
        perror("Buffer error!");
        exit(EXIT_FAILURE);
    }

    memcpy(this->msg, buffer, pos + 1);
    this->msg[pos + 1] = '\0';
    int fd_len = this->size_buffer - pos - 1;
    memcpy(this->fd, buffer + pos + 1, fd_len);
    this->fd[fd_len + 1] = '\0';
}