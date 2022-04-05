#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define DEFAULT_LENGTH 1024
#define DEFAULT_PORT 8888
#define BUFFER_SIZE 5
#define NUM_WORKER 5
#define EXIT_CHAR "exit"
#define SA struct sockaddr
void *main_thread(void *args);
void *worker_thread(void *args);
void *log_thread(void *args);
pthread_mutex_t main_mutex, work_mutex, log_mutex;

int main(int argc, char **argv){
    int i, length, port, work, buf, server_socket;
    char *term;
    struct sockaddr_in server;
    pthread_t tid;

    for(i=0; i<argc; i++){
        if(strcmp(argv[i], "-p") == 0){
            port = atoi(argv[i++]);
            if(port < 1024 || port > 9999){
                printf("server: insufficient port number.\n");
                exit(1);
            }
        }
        else{
            port = DEFAULT_PORT;
        }

        if(strcmp(argv[i], "-l") == 0){
            length = atoi(argv[i++]);
            if(length < 128 || length > 2048){
                printf("server: insufficient message length.\n");
                exit(1);
            }
        }
        else{
            length = DEFAULT_LENGTH;
        }

        if(strcmp(argv[i], "-w") == 0){
            work = atoi(argv[i++]);
            if(work < 1 || port > 10){
                printf("server: insufficient number of workers.\n");
                exit(1);
            }
        }
        else{
            work = NUM_WORKER;
        }

        if(strcmp(argv[i], "-b") == 0){
            buf = atoi(argv[i++]);
            if(buf < 0 || buf > 10){
                printf("server: insufficient buffer size.\n");
                exit(1);
            }
        }
        else{
            buf = BUFFER_SIZE;
        }

        if(strcmp(argv[i], "-t") == 0){
            term = argv[i++];
            if(!term){
                printf("server: insufficient terminator character.\n");
                exit(1);
            }
        }
        else{
            term = EXIT_CHAR;
        }
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
    pthread_mutex_init(&main_mutex, NULL);

    while(1){
        pthread_mutex_lock(&main_mutex);

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

            if(pthread_detach(thread[i]) == -1){
                printf("server: failed to join worker thread.\n");
                exit(1);
            }
        }
        pthread_mutex_unlock(&main_mutex);
    }
}

void *worker_thread(void *args){
    int client_socket = *((int*)args);
    char client_message[DEFAULT_LENGTH];
    pthread_t log;
    pthread_mutex_init(&work_mutex, NULL);
    
	while(1){
        pthread_mutex_lock(&work_mutex);

        if(recv(client_socket, client_message, sizeof(client_message), 0) == -1){
            printf("server: recieve failed\n");
            exit(1);
        }

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
        pthread_mutex_unlock(&work_mutex);
	}
    return NULL;
}

void *log_thread(void *args){
    char *client_message = args;
    time_t cal_time;
    FILE *log;

    cal_time = time(NULL);

    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_lock(&log_mutex);
    
    log = fopen("log.txt", "a");
    if(log == NULL){
        printf("server: log file failed.\n");
    }

    fprintf(log, "%s %s", client_message, asctime(localtime(&cal_time)));

    fclose(log);
    pthread_mutex_unlock(&log_mutex);
    return NULL;
}