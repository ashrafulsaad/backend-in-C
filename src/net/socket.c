#include "server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int socket_create_listener(int domain, int service, int protocol, unsigned long netinterface, int port, int backlog)
{
    int sock = socket(domain, service, protocol);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = domain;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(netinterface);

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    if (listen(sock, backlog) < 0) {
        perror("listen");
        close(sock);
        return -1;
    }

    return sock;
}

void socket_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

void server_send_response(int client_fd, struct HttpResponse *response)
{
    if (!response) {
        return;
    }

    char header_buffer[4096];
    int offset = snprintf(header_buffer, sizeof(header_buffer),
                          "HTTP/1.1 %d %s\r\n"
                          "Server: C-Backend/1.0\r\n"
                          "Connection: close\r\n",
                          response->status_code, response->reason_phrase);
    for (size_t i = 0; i < response->header_count; ++i) {
        offset += snprintf(header_buffer + offset, sizeof(header_buffer) - offset, "%s: %s\r\n", response->headers[i].name, response->headers[i].value);
    }
    if (response->body_length > 0) {
        offset += snprintf(header_buffer + offset, sizeof(header_buffer) - offset, "Content-Length: %zu\r\n", response->body_length);
    }
    offset += snprintf(header_buffer + offset, sizeof(header_buffer) - offset, "\r\n");
    send(client_fd, header_buffer, offset, 0);
    if (response->body_length > 0) {
        send(client_fd, response->body, response->body_length, 0);
    }
}
