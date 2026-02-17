#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "pingpong.h"

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8080
#define BUF_SIZE    32

static int connectToServer(void) {
    int sFd;
    int opt = 1;
    struct sockaddr_in addr;

    sFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sFd < 0) {
        perror("socket");
        return -1;
    }

    setsockopt(sFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sFd);
        return -1;
    }

    if (connect(sFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sFd);
        return -1;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
    return sFd;
}

static int waitForStart(int sFd) {
    char buffer[BUF_SIZE];

    if (send(sFd, "CONNECT\n", 8, 0) < 0)
        return -1;

    printf("Waiting for opponent...\n");

    int n = recv(sFd, buffer, BUF_SIZE - 1, 0);
    if (n <= 0)
        return -1;

    buffer[n] = '\0';

    if (strcmp(buffer, "START") == 0)
        return 0;

    return -1;
}

int main(void) {
    int sFd = connectToServer();
    if (sFd < 0)
        return 1;

    if (waitForStart(sFd) < 0) {
        printf("Server error...\n");
        close(sFd);
        return 1;
    }

    pingpong(sFd);

    close(sFd);
    return 0;
}
