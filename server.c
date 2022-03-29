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
#define BUFFER_SIZE 1024
#define NUM_WORKER 10
#define EXIT_CHAR "x"
#define SA struct sockaddr
void *echo_thread(void *vargp);

int main(int argc, char **argv){
    int i, length, port;
    int socket_desc, new_socket;
    int n = sizeof(struct sockaddr_in);
    struct sockaddr_in server, client;
    pthread_t tid, wthread[NUM_WORKER];

    if(argc == 1){
        length = DEFAULT_LENGTH;
        port = DEFAULT_PORT;
    }

    else if(argc == 2){
        length = atoi(argv[1]);
        port = DEFAULT_PORT;
        if(length < 128 || length > 2048){
            printf("echo: insufficient length value.\n");
            exit(1);
        }
    }

    else if(argc == 3){
        length = atoi(argv[1]);
        port = atoi(argv[2]);

        if(length < 128 || length > 2048){
            printf("echo: insufficient length value.\n");
            exit(1);
        }
        
        if(port < 1024 || port > 9999){
            printf("echo: insufficient port value.\n");
            exit(1);
        }
    }

    else if(argc > 3){
        printf("echo: error, too many parameters.\n");
        exit(1);
    }

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc == -1){
        printf("echo: could not create a socket\n");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    if(bind(socket_desc, (SA*) &server, sizeof(server)) == -1){
        printf("echo: bind failed\n");
        exit(1);
    }
    printf("echo: bind complete\n");

    listen(socket_desc, 3);
    printf("echo: waiting for incoming connections\n");
    
    for(i=0; i<NUM_WORKER; i++){
        if(pthread_create(&wthread[i], NULL, echo_thread, NULL) == -1){
            printf("echo: failed to open thread\n");
            exit(1);
        }
    }

    while(1){
        new_socket = accept(socket_desc, (SA*) &client, (socklen_t*) &n);
        if(new_socket == -1){
            printf("echo: accept failed\n");
            exit(1);
        }
        printf("echo: connection accepted\n");

        pthread_create(&tid, NULL, echo_thread, &new_socket);

        for(i=0; i < NUM_WORKER; i++){
            if(pthread_join(wthread[i], NULL) == -1){
                printf("echo: failed to join thread\n");
                exit(1);
            }
        }
    }
    return 0;
}

void *echo_thread(void *vargp){
    int connfd = *((int*)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    return NULL;
}