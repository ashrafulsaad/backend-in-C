#include "server.h"
#include <stdio.h>
#include<stdlib.h>

struct Server server_constructor(int domain, int service, int protocol,unsigned long netinterface ,int port , int backlog , void (*launch)(struct Server *server))
{
    struct Server server;
    server.domain =domain;
    server.service =service;
    server.protocol=protocol;
    server.netinterface = netinterface;
    server.port = port;
    server.backlog =backlog;

    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr =htonl(netinterface);
    server.socket =socket(domain,service,protocol);
    if(server.socket < 0){
        perror("Failed to connnect socket...\n");
        exit(1);
    }

    if(bind(server.socket, (struct sockaddr *)&server.address,sizeof(server.address))<0)
    {
        perror("Failed to bind socket...\n");
        exit(1);
    }

    if((listen(server.socket,server.backlog)) <0)
    {
        perror("Failed to start listening...\n");
        exit(1);
    }
    server.launch=launch;
    return server;
}
