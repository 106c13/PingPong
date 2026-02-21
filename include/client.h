#ifndef CLIENT_H
#define CLIENT_H

#include "buffer.h"
#include "server.h"


typedef struct s_server Server;
typedef struct s_game   Game;


typedef struct s_client {
    int                 fd;
	int					y;
    Buffer              sendBuffer;
    Buffer              recvBuffer;

    Game*               game;
} Client;

/* client_utils.c */
Client* getClientByFd(Server* server, int fd);
void    deleteClient(Server* server, int fd);
void    addClient(Server* server, int fd);

#endif
