#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define DEFAULT_LENGTH 1024
#define DEFAULT_PORT 8888
#define SA struct sockaddr

int main(int argc, char **argv){
    int server_socket, length, port;
    struct sockaddr_in server;
    //if client takes no arguments.
    if(argc == 1){
        //set length and port to default.
        length = DEFAULT_LENGTH;
        port = DEFAULT_PORT;
    }
    //if client takes one argument.
    else if(argc == 2){
        //set length to the argument and port to default.
        length = atoi(argv[1]);
        port = DEFAULT_PORT;
    }
    //if client takes two arguments.
    else if(argc == 3){
        //set length to first argument and port to second argument.
        length = atoi(argv[1]);
        port = atoi(argv[2]);
    }
    //if client takes more than two arguments.
    else if(argc > 3){
        //printf error message and exit.
        printf("client: error too many arguments.\n");
        exit(1);
    }

    char client_message[length], server_message[length];
    //setup the socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("client: could not create a socket.\n");
    }
    //setup the server.
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    //connect the socket to the server.
    if(connect(server_socket, (SA*) &server, sizeof(server)) == -1){
        printf("client: connect failed.\n");
        exit(1);
    }
    printf("client: connected.\n");
    sleep(1);
    
    while(1){
        //get message from client
        printf("client: enter a message to server: ");
        fgets(client_message, length, stdin);
        //account for spaces in message.
        if((strlen(client_message) > 0) && (client_message[strlen(client_message) - 1] == '\n')){
            client_message[strlen(client_message) - 1] = '\0';
        }
        //send the message to the server.
        if(send(server_socket, client_message, strlen(client_message), 0) == -1){
            printf("client: send failed.\n");
            break;
        }
        printf("client: data sent.\n");
        sleep(1);
        //recieve the message back from the server.
        if(recv(server_socket, server_message, sizeof(server_message), 0) == -1){
            printf("client: recieve failed.\n");
            break;
        }
        //if the message was terminator character.
        if(strcmp(server_message, "server: thank you for using echo server.") == 0){
            //print exit message from server and break out of loop.
            puts(server_message);
            break;
        }
        //else print success message and print the server message.
        printf("client: reply received.\n");
        sleep(1);
        puts(server_message);
        sleep(1);
    }
    return 0;
}