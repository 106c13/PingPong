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

int main() {
    int sFd;
    int opt = 1;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];

    sFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sFd < 0) {
        write(STDERR_FILENO, "Failed to create socket\n", 24);
        return 1;
    }

    setsockopt(sFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sFd);
        return 1;
    }

    if (connect(sFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect failed");
        close(sFd);
        return 1;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
   
    printf("Waiting for oponent...\n");
    if (recv(sFd, buffer, BUF_SIZE - 1, 0) < 0) {
        printf("Server error...\n");
        close(sFd);
        return 1;
    }
    
    if (strcmp(buffer, "START") == 0) {
        pingpong(sFd); //acctual game
    }

    close(sFd);
    return 0;
}
