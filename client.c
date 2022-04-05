#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define SA struct sockaddr

int main(int argc, char **argv){
    int server_socket;
    struct sockaddr_in server;
    char client_message[1024], server_message[1024];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("client: could not create a socket\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if(connect(server_socket, (SA*) &server, sizeof(server)) == -1){
        printf("client: connect failed.\n");
        exit(1);
    }
    printf("client: connected.\n");
    sleep(1);
    
    while(1){
        printf("client: enter a message to server: ");
        scanf("%s", client_message);

        if(send(server_socket, client_message, strlen(client_message), 0) == -1){
            printf("client: send failed.\n");
            exit(1);
        }
        printf("client: data sent.\n");
        sleep(1);
        
        if(recv(server_socket, server_message, sizeof(server_message), 0) == -1){
            printf("client: recieve failed.\n");
            exit(1);
        }

        if(strcmp(server_message, "server: thank you for using echo server.\n") == 0){
            puts(server_message);
            exit(1);
        }

        printf("client: reply received.\n");
        sleep(1);
        puts(server_message);
        sleep(1);
    }
    return 0;
}