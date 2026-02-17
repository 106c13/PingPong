#ifndef CLIENT_H
#define CLIENT_H

#include "buffer.h"

typedef struct s_client {
    int                 fd;
    Buffer              sendBuffer;
    Buffer              recvBuffer;
    struct s_client*    opponent;
} Client;

#endif
