#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include "server.h"
#include "client.h"

long long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

Client* getOpponent(Client* client) {
    Game* g = client->game;
    if (!g)
        return NULL;

    return (g->p1 == client) ? g->p2 : g->p1;
}

void processGames(Game** games, int size) {
    long long now = get_time_ms();

    for (int i = 0; i < size; i++) {
        Game* game = games[i];
        Ball* ball = &game->ball;

        float dt = (now - game->lastUpdate) / 10;
        if (dt == 0)
            continue;

        game->lastUpdate = now;

        ball->x += ball->vx * dt;
        ball->y += ball->vy * dt;
        
        if (ball->y - BALL_RADIUS < 0) {
            ball->y = 10 + BALL_RADIUS;
            ball->vy *= -1;
        } else if (ball->y + BALL_RADIUS > 600) {
            ball->y = 600 - BALL_RADIUS;
            ball->vy *= -1;
        }

        if (ball->x - BALL_RADIUS < 0 || ball->x + BALL_RADIUS > 900) {
            ball->x = 450;
            ball->y = 300;
            ball->vx *= -1;
        }

        int p1x = 5;
        int p1y = game->p1->y;

        if (ball->x - BALL_RADIUS <= p1x + PADDLE_WIDTH &&
            ball->x > p1x &&
            ball->y >= p1y &&
            ball->y <= p1y + PADDLE_HEIGHT)
        {
            ball->x = p1x + PADDLE_WIDTH + BALL_RADIUS + 1;
            ball->vx *= -1;

            //float hitPos = (ball->y - p1y) / (float)PADDLE_HEIGHT;
            //ball->vy = (hitPos - 0.5f) * 600;
        }

        int p2x = 885;
        int p2y = game->p2->y;

        if (ball->x + BALL_RADIUS >= p2x &&
            ball->x < p2x + PADDLE_WIDTH &&
            ball->y >= p2y &&
            ball->y <= p2y + PADDLE_HEIGHT)
        {
            ball->x = p2x - BALL_RADIUS - 1;
            ball->vx *= -1;

            //float hitPos = (ball->y - p2y) / (float)PADDLE_HEIGHT;
            //ball->vy = (hitPos - 0.5f) * 600;
        }
    }
}

static void updateEpollEvents(Server* server, int fd, bool write) {
    struct epoll_event ev;
    ev.events = EPOLLIN | (write ? EPOLLOUT : 0);
    ev.data.fd = fd;
    epoll_ctl(server->epollFd, EPOLL_CTL_MOD, fd, &ev);
}

static void flushSendBuffer(Server* server, Client* client) {
    Buffer* buf = &client->sendBuffer;
    int len = bufferSize(buf);
    if (len == 0)
        return;

    //printf("BUFFER (%d): %s\n", client->fd, buf->data + buf->start);
    int n = write(client->fd, buf->data + buf->start, len);
    if (n > 0)
        consume(buf, n);

    //printf("%d BUFFER SIZE: %d\n", client->fd, bufferSize(buf));
    updateEpollEvents(server, client->fd, bufferSize(buf) > 0);
}

static void sendToClient(Server* server, Client* client, const char* msg) {
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

    Game* game = malloc(sizeof(Game));
    if (!game)
        return sendToClient(server, c1, "ERROR\n"), sendToClient(server, c2, "ERROR\n");

    game->p1 = c1;
    game->p2 = c2;
    game->ball.x = 450;
    game->ball.y = 300;
    game->ball.vx = 2;
    game->ball.vy = 2;
    game->lastUpdate = get_time_ms();
    server->games[server->gameCount++] = game;

	c1->y = 250;
	c2->y = 250;
    c1->game = game;
    c2->game = game;

	char buf[32];
	snprintf(buf, sizeof(buf), "START:%d:%d:450:300\n", c1->y, c2->y);

    sendToClient(server, c1, buf);
    sendToClient(server, c2, buf);

    printf("Create new game...\n");
}

static void acceptClient(Server* server) {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = accept(server->servFd,
                          (struct sockaddr*)&clientAddr,
                          &addrLen);

    if (clientFd < 0)
        return;

    if (server->clientCount == MAX_CLIENTS) {
        send(clientFd, "FULL\n", 5, MSG_DONTWAIT);
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

static void finishGame(Server* server, Game* game, Client* client) {
    if (!game)
        return;

    if (game->p1 == client)
        sendToClient(server, game->p2, "WIN\n");
    else
        sendToClient(server, game->p1, "WIN\n");

    game->p1->game = NULL;
    game->p2->game = NULL;
    

    bool found = false;
    for (int i = 0; i < server->gameCount; i++) {
        if (server->games[i] == game)
            found = true;

        if (found && i + 1 < server->gameCount)
            server->games[i] = server->games[i + 1];
    }
    if (found)
        server->games[--server->gameCount] = NULL;
    free(game);
}

static void processEvent(struct epoll_event* event, Server* server) {
    int fd = event->data.fd;

    if (fd == server->servFd)
        return acceptClient(server);

    Client* client = getClientByFd(server, fd);

    if (event->events & EPOLLIN) {
        char buf[1024];
        int n = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (n <= 0) {
            close(fd);
            epoll_ctl(server->epollFd, EPOLL_CTL_DEL, fd, NULL);

            finishGame(server, client->game, client);
            deleteClient(server, fd);
            return;
        }

        buf[n] = '\0';

        printf("Received (%d): %.*s", fd, n, buf);

        if (strcmp(buf, "CONNECT\n") == 0) {
            addClient(server, fd);
            if (server->clientCount > 0 && server->clientCount % 2 == 0)
                startGame(server);
        } else if (client) {
            Game* game = client->game;
            
            if (game) {
                Client* opponent = getOpponent(client);
                int x = game->ball.x;
                int y = game->ball.y;

                client->y = (int)strtol(buf, NULL, 10);

                if (game->p1 != client)
                    x = 900 - x;

                snprintf(buf, sizeof(buf), "%d:%d:%d:%d\n", client->y, opponent->y, x, y);
                sendToClient(server, client, buf);
            }

		}
    }
    
    if (event->events & EPOLLOUT)
        flushSendBuffer(server, client);
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

        processGames(server->games, server->gameCount);
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
    if (eFd < 0)
        return NULL;

    Server* server = malloc(sizeof(Server));
    server->clientCount = 0;
    server->gameCount = 0;
    server->servFd = sFd;
    server->epollFd = eFd;

    return server;
}

void exitServer(Server* server) {
    close(server->servFd);
    free(server);
}
