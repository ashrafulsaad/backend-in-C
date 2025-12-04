#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct Server
{
    int domain;
    int service;
    int protocol;
    unsigned long netinterface;
    int port;
    int backlog;

    struct sockaddr_in address;

    int socket;  // renamed from socket

    void (*launch)(struct Server *server);
};

struct Server server_constructor(
    int domain,
    int service,
    int protocol,
    unsigned long netinterface,
    int port,
    int backlog,
    void (*launch)(struct Server *server)
);

#endif
