#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SA struct sockaddr

int main(int argc, char **argv){
    int socket_desc;
    struct sockaddr_in server;
    char *message = "\0", server_reply[1024];

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc == -1){
        printf("echo: could not create a socket\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("74.125.235.20");
    server.sin_port = htons(80);

    if(connect(socket_desc, (SA*) &server, sizeof(server)) == -1){
        printf("echo: connect failed\n");
        exit(1);
    }
    printf("echo: connected\n");
    printf("echo: enter a message to echo server: ");
    scanf("%s", message);

    if(send(socket_desc, message, strlen(message), 0) == -1){
        printf("echo: send failed\n");
        exit(1);
    }
    printf("echo: data sent\n");
    
    if(recv(socket_desc, server_reply, sizeof(server_reply), 0) == -1){
        printf("echo: recv failed\n");
    }

    printf("echo: reply received\n");
    puts(server_reply);
    return 0;
}