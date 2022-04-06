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
#define SA struct sockaddr
void *main_thread(void *args);
void *worker_thread(void *args);
void *log_thread(void *args);
void parse_cmdline(int argc, char **argv);
int length, port, work, buf;
char *term;
pthread_mutex_t main_mutex, work_mutex, log_mutex;

int main(int argc, char **argv){
    int server_socket;
    struct sockaddr_in server;
    pthread_t tid;
    //parse the command line to extract command line values.
    parse_cmdline(argc, argv);
    //setup the socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("server: could not create a socket.\n");
    }
    printf("server: socket created\n");
    sleep(1);
    //setup the server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    //bind the socket to the server.
    if(bind(server_socket, (SA*) &server, sizeof(server)) == -1){
        printf("server: bind failed.\n");
        exit(1);
    }
    printf("server: bind complete.\n");
    sleep(1);
    //listen for client connections.
    if(listen(server_socket, 3) == -1){
        printf("server: listen failed\n");
        exit(1);
    }
    printf("server: listening for connections...\n");
    //open the main thread.
    if(pthread_create(&tid, NULL, main_thread, &server_socket) == -1){
        printf("server: failed to open main thread.\n");
        exit(1);
    }
    //join the main thread.
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
    pthread_t thread[work];
    pthread_mutex_init(&main_mutex, NULL);

    while(1){
        //lock the main thread.
        pthread_mutex_lock(&main_mutex);
        //accept the client socket;
        client_socket = accept(server_socket, (SA*) &client, (socklen_t*) &size);
        if(client_socket == -1){
            printf("server: accept failed.\n");
            exit(1);
        }
        printf("server: connection accepted.\n");
        //open all the worker threads.
        for(i=0; i<work; i++){
            if(pthread_create(&thread[i], NULL, worker_thread, &client_socket) == -1){
                printf("server: failed to open worker thread.\n");
                exit(1);
            }
            //detatch worker threads for concurrency.
            if(pthread_detach(thread[i]) == -1){
                printf("server: failed to join worker thread.\n");
                exit(1);
            }
        }
        //unlock the main thread.
        pthread_mutex_unlock(&main_mutex);
    }
}

void *worker_thread(void *args){
    int client_socket = *((int*)args);
    char client_message[length];
    pthread_t log;
    pthread_mutex_init(&work_mutex, NULL);
    
	while(1){
        //lock the worker thread.
        pthread_mutex_lock(&work_mutex);
        //recieve message from client.
        if(recv(client_socket, client_message, sizeof(client_message), 0) == -1){
            printf("server: recieve failed\n");
            exit(1);
        }
        //check if message is the terminating character.
		if(strcmp(client_message, term) == 0){
            //send an exit message to client.
            if(send(client_socket, "server: thank you for using echo server.", 100, 0) == -1){
                printf("server: send failed.\n");
            }
        }
        //send the message back to the client.
		else if(send(client_socket, client_message, sizeof(client_message), 0) == -1){
            printf("server: send failed.\n");
            exit(1);
        }
        //open the log thread.
        if(pthread_create(&log, NULL, log_thread, &client_message) == -1){
            printf("server: failed to open log thread\n");
            exit(1);
        }
        //unlock the worker thread.
        pthread_mutex_unlock(&work_mutex);
	}
}

void *log_thread(void *args){
    char *client_message = args;
    time_t cal_time;
    FILE *log;

    cal_time = time(NULL);
    pthread_mutex_init(&log_mutex, NULL);
    //lock the log thread.
    pthread_mutex_lock(&log_mutex);
    //open log file for storing log information.
    log = fopen("log.txt", "a");
    if(log == NULL){
        printf("server: log file failed.\n");
    }
    //print the contents of the client message with respected calendar time to the log file.
    fprintf(log, "%s %s", client_message, asctime(localtime(&cal_time)));
    //close the log and unlock the log thread.
    fclose(log);
    pthread_mutex_unlock(&log_mutex);
    return NULL;
}

void parse_cmdline(int argc, char **argv){
    int i;
    int p = 0, l = 0, w = 0, b = 0, t = 0;
    //iterate through all argv commands.
    for(i=0; i<argc; i++){
        //if argv contains "-l" flag.
        if(strcmp(argv[i], "-l") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter the number of workers.\n");
                exit(1);
            }
            //set the argument after flag to length.
            length = atoi(argv[i+1]);
            //check if the value is sufficient.
            if(length < 128 || length > 2048){
                printf("server: insufficient message length.\n");
                exit(1);
            }
            //increment port flag.
            l++;
        }
        //if argv contains "-p" flag.
        else if(strcmp(argv[i], "-p") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter the number of workers.\n");
                exit(1);
            }
            //set the argument after flag to port.
            port = atoi(argv[i+1]);
            //check if the value is sufficient.
            if(port < 1024 || port > 9999){
                printf("server: insufficient port number.\n");
                exit(1);
            }
            //increment port flag.
            p++;
        }
        //if argv contains "-w" flag.
        else if(strcmp(argv[i], "-w") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter the number of workers.\n");
                exit(1);
            }
            //set the argument after flag to work.
            work = atoi(argv[i+1]);
            //check if the value is sufficient.
            if(work < 1 || work > 10){
                printf("server: insufficient number of workers.\n");
                exit(1);
            }
            //increment port flag.
            w++;
        }
        //if argv contains "-b" flag.
        else if(strcmp(argv[i], "-b") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter a buffer size\n");
                exit(1);
            }
            //set the argument after flag to buf.
            buf = atoi(argv[i+1]);
            //check if the value is sufficient.
            if(buf < 1 || buf > 10){
                printf("server: insufficient buffer size.\n");
                exit(1);
            }
            //increment port flag.
            b++;
        }
        //if argv contains "-t" flag.
        else if(strcmp(argv[i], "-t") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter a terminator character.\n");
                exit(1);
            }
            //set the argument after flag to term.
            term = argv[i+1];
            //check if the value is sufficient.
            if(!term){
                printf("server: insufficient terminator character.\n");
                exit(1);
            }
            //increment port flag.
            t++;
        }
    }
    //if length is not given by user. 
    if(l == 0){
        //set length to default.
        length = DEFAULT_LENGTH;
    }
    //if port is not given by user.
    if(p == 0){
        //set port to default.
        port = DEFAULT_PORT;
    }
    //if work, buf, or terminator is not given by user.
    if(w == 0 && b == 0 && t == 0){
        //print error message and exit.
        printf("server: please enter the number of workers, buf, and terminator character\n");
        exit(1);
    }
}