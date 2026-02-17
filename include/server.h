#ifndef SERVER_H
#define SERVER_H

#include <sys/epoll.h>
#include "client.h"

#define MAX_CLIENTS 4

/* PingPongProtocol (PPP)
 * 
 * CONNECT client searching a new game
 * READY client is ready to play
 * UP DOWN player making moves
 * EXIT player exits from the game
 *
 * When both clients are connected, both should send READY, to start the game.
 * Everytime whem player moves, he will send UP or DOWN message.
 * If player exits from the game, he will send EXIT message.
 *
 * <GAME_ID>:<PLAYER_ID>:<ACTION>:<VALUE>
*/

typedef struct s_server {
    int     servFd;
    int     epollFd;
    int     clientCount;
    Client* clients[MAX_CLIENTS];
} Server;

/* server.c */
Server* initServer(int port);
void    exitServer(Server* server);
void    loopServer(Server* server);

void    acceptClient(Server* server);

#endif
