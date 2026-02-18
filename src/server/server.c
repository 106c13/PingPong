#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <stdio.h>
#include "server.h"
#include "client.h"

Client* getClientByFd(Server* server, int fd) {
    for (int i = 0; i < server->clientCount; i++) {
        if (server->clients[i]->fd == fd)
            return server->clients[i];
    }
    return NULL;
}

void exitServer(Server* server) {
    close(server->servFd);
    free(server);
}

void deleteClient(Server* server, int fd) {
    if (server->clientCount == 0)
        return;

    printf("Deleteing client...\n");

    bool found = false;
    for (int i = 0; i < server->clientCount; i++) {
        if (server->clients[i]->fd == fd) {
            found = true;
            close(server->clients[i]->fd);
            free(server->clients[i]);
        }

        if (found && i + 1 < server->clientCount) {
            server->clients[i] = server->clients[i + 1];
        }
    }

    if (found) {
        server->clientCount--;
        server->clients[server->clientCount] = NULL;
    }
}

Server* initServer(int port) {
    int sFd;
    int opt = 1;
    struct sockaddr_in addr;

    sFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sFd < 0) {
        write(STDERR_FILENO, "Failed to create socket\n", 24);
        return NULL;
    }

    setsockopt(sFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        write(STDERR_FILENO, "Failed to bind\n", 15);
        close(sFd);
        return NULL;
    }

    if (listen(sFd, 10) < 0) {
        close(sFd);
        return NULL;
    }

    int eFd = epoll_create1(0);
    if (eFd < 0) {
        return NULL;
    }

    Server* server = malloc(sizeof(Server));
    server->servFd = sFd;
    server->epollFd = eFd;

    return server;
}


void updateEpollEvents(Server* server, int fd, bool write) {
    struct epoll_event ev;
    ev.events = EPOLLIN | (write ? EPOLLOUT : 0);
    ev.data.fd = fd;
    epoll_ctl(server->epollFd, EPOLL_CTL_MOD, fd, &ev);
}

void flushSendBuffer(Server* server, Client* client) {
    Buffer* buf = &client->sendBuffer;
    int len = bufferSize(buf);
    if (len == 0)
        return;

    //printf("BUFFER (%d): %s\n", client->fd, buf->data + buf->start);
    int n = write(client->fd, buf->data + buf->start, len);
    if (n > 0) {
        consume(buf, n);
    }
    //printf("%d BUFFER SIZE: %d\n", client->fd, bufferSize(buf));
    updateEpollEvents(server, client->fd, bufferSize(buf) > 0);
}

void sendToClient(Server* server, Client* client, const char* msg) {
    append(&client->sendBuffer, msg);

    // Enable EPOLLOUT so epoll will notify us to flush
    //printf("Client fd: %d %s\n", client->fd, client->sendBuffer.data);
    updateEpollEvents(server, client->fd, true);
}

static void startGame(Server* server) {
    if (server->clientCount < 2)
        return;

    if (server->clientCount % 2 != 0)
        return;

    Client* c1 = server->clients[server->clientCount - 1];
    Client* c2 = server->clients[server->clientCount - 2];

    c1->opponent = c2;
    c2->opponent = c1;
	c1->y = 250;
	c2->y = 250;

	char buf[32];

	snprintf(buf, sizeof(buf), "START:%d:%d:300:300\n", c1->y, c2->y);
    sendToClient(server, c1, buf);
    sendToClient(server, c2, buf);
}

void addClient(Server* server, int fd) {
    Client* newClient = malloc(sizeof(Client));
    
    if (!newClient)
        return;

    newClient->fd = fd;
    initBuffer(&newClient->sendBuffer, 32);
    initBuffer(&newClient->recvBuffer, 32);

    server->clients[server->clientCount++] = newClient;

    printf("New player connected...\n");

    if (server->clientCount > 0 && server->clientCount % 2 == 0)
        startGame(server);
}

void processEvent(struct epoll_event* event, Server* server) {
    int fd = event->data.fd;

    if (fd == server->servFd) {
        acceptClient(server);
        return;
    }

    Client* client = getClientByFd(server, fd);

    if (event->events & EPOLLIN) {
        char buf[1024];
        int n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (n <= 0) {
            close(fd);
            epoll_ctl(server->epollFd, EPOLL_CTL_DEL, fd, NULL);
            deleteClient(server, fd);
            return;
        }

        buf[n] = '\0';

        printf("Received (%d): %.*s", fd, n, buf);

        if (strcmp(buf, "CONNECT\n") == 0)
            addClient(server, fd);
        else if (client) {
			client->y = (int)strtol(buf, NULL, 10);
			snprintf(buf, sizeof(buf), "%d:%d:%d:%d\n", client->opponent->y, client->y, client->y, client->y);
            sendToClient(server, client->opponent, buf);
		}
    }
    
    if (event->events & EPOLLOUT) {
        flushSendBuffer(server, client);
    }
}

void acceptClient(Server* server) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = accept(server->servFd,
                          (struct sockaddr*)&clientAddr,
                          &addrLen);

    if (clientFd < 0)
        return;

    if (server->clientCount == MAX_CLIENTS) {
        send(clientFd, "-1", 2, MSG_DONTWAIT);
        close(clientFd);
        return;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

    printf("Got connection from %s:%d\n",
           ip,
           ntohs(clientAddr.sin_port));

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;

    epoll_ctl(server->epollFd, EPOLL_CTL_ADD, clientFd, &ev);
}

void loopServer(Server* server) {
    struct epoll_event events[10];
    struct epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = server->servFd;
    epoll_ctl(server->epollFd, EPOLL_CTL_ADD, server->servFd, &ev);

    while (true) {
        int evCount = epoll_wait(server->epollFd, events, 10, -1);

        for (int i = 0; i < evCount; i++) {
            processEvent(&events[i], server);
        }
    }
}
