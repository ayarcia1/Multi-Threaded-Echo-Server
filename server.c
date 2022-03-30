#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define DEFAULT_LENGTH 1024
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define NUM_WORKER 10
#define EXIT_CHAR "x"
#define SA struct sockaddr
void *echo_thread(void *vargp);
void *log_thread(void *vargp);

int main(int argc, char **argv){
    int i, length, port;
    int server_socket, client_socket;
    int size = sizeof(struct sockaddr_in);
    struct sockaddr_in server, client;
    pthread_t tid, log, wthread[NUM_WORKER];

    if(argc == 1){
        length = DEFAULT_LENGTH;
        port = DEFAULT_PORT;
    }

    else if(argc == 2){
        length = atoi(argv[1]);
        port = DEFAULT_PORT;
        if(length < 128 || length > 2048){
            printf("server: insufficient length value.\n");
            exit(1);
        }
    }

    else if(argc == 3){
        length = atoi(argv[1]);
        port = atoi(argv[2]);

        if(length < 128 || length > 2048){
            printf("server: insufficient length value.\n");
            exit(1);
        }
        
        if(port < 1024 || port > 9999){
            printf("server: insufficient port value.\n");
            exit(1);
        }
    }

    else if(argc > 3){
        printf("server: error, too many parameters.\n");
        exit(1);
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("server: could not create a socket.\n");
    }
    printf("server: socket created\n");
    sleep(1);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    if(bind(server_socket, (SA*) &server, sizeof(server)) == -1){
        printf("server: bind failed.\n");
        exit(1);
    }
    printf("server: bind complete.\n");
    sleep(1);

    if(listen(server_socket, 3) == -1){
        printf("server: listen failed.\n");
        exit(1);
    }
    printf("server: waiting for incoming connections.\n");
    sleep(1);

    pthread_create(&log, NULL, log_thread, &client_socket);
    
    for(i=0; i<NUM_WORKER; i++){
        if(pthread_create(&wthread[i], NULL, echo_thread, NULL) == -1){
            printf("server: failed to open thread.\n");
            exit(1);
        }
    }

    while(1){
        printf("server: listening...");
        sleep(1);

        client_socket = accept(server_socket, (SA*) &client, (socklen_t*) &size);
        if(client_socket == -1){
            printf("server: accept failed.\n");
            exit(1);
        }
        printf("server: connection accepted.\n");
        sleep(1);

        pthread_create(&tid, NULL, echo_thread, &client_socket);

        for(i=0; i < NUM_WORKER; i++){
            if(pthread_join(wthread[i], NULL) == -1){
                printf("server: failed to join thread.\n");
                exit(1);
            }
        }
    }
    return 0;
}

void *echo_thread(void *vargp){
    int connect_socket = *((int*)vargp);
    int recieve;
	char message[BUFFER_SIZE];

	while((recieve = recv(connect_socket, message, BUFFER_SIZE, 0)) != -1){
		message[recieve] = '\0';
		if(strcmp(message, EXIT_CHAR) == 0){
            break;
        }
		send(connect_socket, message, strlen(message), 0);	
	}
    free(vargp);
    return NULL;
}

void *log_thread(void *vargp){
    FILE *n;
    n = fopen("log.txt", "a");
    if(n == NULL){
        printf("server: log file failed.\n");
    }
    return NULL;
}