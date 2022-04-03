#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#define DEFAULT_LENGTH 1024
#define DEFAULT_PORT 8888
#define BUFFER_SIZE 1024
#define NUM_WORKER 5
#define EXIT_CHAR "x"
#define SA struct sockaddr
void *main_thread(void *args);
void *worker_thread(void *args);
void *log_thread(void *args);

int main(int argc, char **argv){
    int length, port;
    int server_socket;
    struct sockaddr_in server;
    pthread_t tid;

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
        printf("server: listen failed\n");
        exit(1);
    }
    printf("server: listening for connections...\n");
    
    if(pthread_create(&tid, NULL, main_thread, &server_socket) == -1){
        printf("server: failed to open main thread.\n");
        exit(1);
    }

    if(pthread_join(tid, NULL) == -1){
        printf("server: failed to join main thread.\n");
        exit(1);
    }

    return 0;
}

void *main_thread(void *args){
    int server_socket = *((int*)args);
    int i, client_socket;
    struct sockaddr_in client;
    int size = sizeof(struct sockaddr_in);
    pthread_t thread[NUM_WORKER];

    while(1){
        client_socket = accept(server_socket, (SA*) &client, (socklen_t*) &size);
        if(client_socket == -1){
            printf("server: accept failed.\n");
            exit(1);
        }
        printf("server: connection accepted.\n");

        for(i=0; i<NUM_WORKER; i++){
            if(pthread_create(&thread[i], NULL, worker_thread, &client_socket) == -1){
                printf("server: failed to open worker thread.\n");
                exit(1);
            }
        }

        for(i=0; i<NUM_WORKER; i++){
            if(pthread_join(thread[i], NULL) == -1){
                printf("server: failed to join worker thread.\n");
                exit(1);
            }
        }
    }
}

void *worker_thread(void *args){
    int client_socket = *((int*)args);
    char client_message[BUFFER_SIZE];
    pthread_t log;

	while(recv(client_socket, client_message, sizeof(client_message), 0) != -1){
		if(strcmp(client_message, EXIT_CHAR) == 0){
            if(send(client_socket, "server: thank you for using echo server.", 100, 0) == -1){
                exit(1);
            }
        }

		else if(send(client_socket, client_message, sizeof(client_message), 0) == -1){
            printf("server: send failed.\n");
            exit(1);
        }

        if(pthread_create(&log, NULL, log_thread, &client_message) == -1){
            printf("server: failed to open log thread\n");
            exit(1);
        }
	}
    return NULL;
}

void *log_thread(void *args){
    char *client_message = args;
    time_t cal_time;
    FILE *log;

    cal_time = time(NULL);
    
    log = fopen("log.txt", "a");
    if(log == NULL){
        printf("server: log file failed.\n");
    }

    fprintf(log, "%s %s\n", client_message, asctime(localtime(&cal_time)));

    fclose(log);
    return NULL;
}