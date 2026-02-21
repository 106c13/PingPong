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
    struct sockaddr_in addr;

    sFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sFd < 0) {
        perror("socket");
        return -1;
    }

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	setsockopt(sFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

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

static int recvPositions(char* buf, int* pos) {
	if (!buf || *buf != ':')
		return -1;

	int i = 0;

	while (i < 4) {
		pos[i++] = (int)strtol(buf + 1, &buf, 10);
        printf("GOT: %d\n", pos[i - 1]);

		if (*buf == '\n')
			break;

		if (*buf != ':')
			return -1;

	}
	return 0;
}

static int waitForStart(int sFd, int* pos) {
	char buf[BUF_SIZE];
    int total = 0;
    
	if (send(sFd, "CONNECT\n", 8, 0) < 0)
		return -1;

    // =========== setTimout(int sec) ===========
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(sFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // ==========================================

	printf("Waiting for opponent...\n");

	buf[0] = '\0';
    while (!strchr(buf, '\n')) {
        int n = recv(sFd, buf + total, BUF_SIZE, 0);

	    if (n <= 0)
		    return -1;

        total += n;
        if (total >= BUF_SIZE)
            return -1;
    }

    tv.tv_sec = 1;
    setsockopt(sFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    printf("Recv: %s\n", buf);

	if (strncmp(buf, "START", 5) == 0)
	    return recvPositions(strchr(buf, ':'), pos);
    else if (strncmp(buf, "FULL", 4) == 0)
        printf("Server is full\n");
    else
        printf("Server error\n");

    return -1;
}

int main(void) {
    int sFd = connectToServer();
	int pos[4];

    if (sFd < 0)
        return 1;

    if (waitForStart(sFd, pos) == 0)
        pingpong(sFd, pos);

    close(sFd);
    return 0;
}
