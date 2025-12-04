#include <unistd.h>
#include<stdio.h>
#include "server.h"
#include<string.h>

void launch(struct Server *server){
    char buffer[30000];
    char *hello = "HTTP/1.1 200 OK\r\n"
    "Date: Mon, 01 Jan 2025 10:00:00 GMT\r\n"
    "Server: C-Custom-Server/1.0\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, World!";
    int address_length = sizeof(server->address);
    int new_socket;

    while(1){
    
    printf("====== WAITING FOR CONNECTION===\n");

    new_socket = accept(server->socket,(struct sockaddr *)&server->address,(socklen_t *)&address_length);
    read(new_socket,buffer,30000);
    printf("%s\n",buffer); 

    write(new_socket,hello,strlen(hello));
    close(new_socket);
    }
}
int main(){
    printf("1\n");
    struct Server server = server_constructor(AF_INET,SOCK_STREAM,0,INADDR_ANY,80,10,launch);
    server.launch(&server);

}