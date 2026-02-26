#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "buffer.h"

void addClient(Server* server, int fd) {
    Client* newClient = malloc(sizeof(Client));
    
    if (!newClient)
        return;

    newClient->fd = fd;
    newClient->game = NULL;
    initBuffer(&newClient->sendBuffer, 32);
    initBuffer(&newClient->recvBuffer, 32);

    server->clients[server->clientCount++] = newClient;

    printf("New player connected...\n");
}

void deleteClient(Server* server, int fd) {
    if (server->clientCount == 0)
        return;

    bool found = false;
    for (int i = 0; i < server->clientCount; i++) {
        if (server->clients[i]->fd == fd) {
            found = true;
            close(server->clients[i]->fd);
            free(server->clients[i]);
        }

        if (found && i + 1 < server->clientCount)
            server->clients[i] = server->clients[i + 1];
    }

    if (found)
        server->clients[--server->clientCount] = NULL;
}

Client* getClientByFd(Server* server, int fd) {
    for (int i = 0; i < server->clientCount; i++) {
        if (server->clients[i]->fd == fd)
            return server->clients[i];
    }
    return NULL;
}
