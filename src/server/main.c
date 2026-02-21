#include <stdio.h>
#include "server.h"

int main() {
    Server* server = initServer(8080);

    if (!server)
        return 1;

    printf("Server started on port %d...\n", 8080);

    loopServer(server);

    exitServer(server);

    return 0;
}
