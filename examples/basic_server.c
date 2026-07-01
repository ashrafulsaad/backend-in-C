#include <stdio.h>
#include "server.h"

void launch(struct Server *server)
{
    server_launch(server);
}

int main(void)
{
    struct Server *server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 128, launch);
    if (!server) {
        return 1;
    }

    server->launch(server);
    server_destroy(server);
    return 0;
}
