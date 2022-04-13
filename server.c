//Name: Arif Ayarci
//Project 3: Multi-Threaded Echo Server
//Date: 4/13/2022
//Section: CIS-3207-01
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
pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char **argv){
    pthread_t main_tid;

    //parse the command line to extract control parameters.
    parse_cmdline(argc, argv);

    //create the main thread.
    if(pthread_create(&main_tid, NULL, main_thread, NULL) == -1){
        printf("server: failed to open main thread.\n");
        exit(1);
    }

    //join the main thread.
    if(pthread_join(main_tid, NULL) == -1){
        printf("server: failed to join main thread.\n");
        exit(1);
    }
    return 0;
}

void *main_thread(void *args){
    int i, j, server_socket, client_socket, size = sizeof(struct sockaddr_in);
    struct sockaddr_in server, client;
    pthread_t work_tid[work];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    //setup the socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        printf("server: could not create a socket.\n");
    }
    printf("server: socket created.\n");
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
        printf("server: listen failed.\n");
        exit(1);
    }
    printf("server: listening for connections...\n");

    while(1){
        //accept the client socket;
        client_socket = accept(server_socket, (SA*) &client, (socklen_t*) &size);
        if(client_socket == -1){
            printf("server: accept failed.\n");
            close(client_socket);
        }
        printf("server: connection accepted.\n");
        
        //open all the worker threads.
        if(pthread_create(&work_tid[i++], NULL, worker_thread, &client_socket) == -1){
            printf("server: failed to open worker thread.\n");
            exit(1);
        }
        
        //after the worker threads have been created.
        if(i >= work){
            i = 0;
            while(i < work){
                //join worker threads.
                if(pthread_join(work_tid[i++], NULL) == -1){
                    printf("server: failed to join worker thread.\n");
                    exit(1);
                }
            }
            i--;

            //lock the main thread.
            pthread_mutex_lock(&mutex);

            //wait for a worker thread to exit.
            while(i >= work){
                j++;
                pthread_cond_wait(&cond, &mutex);
            }

            //unlock the main thread.
            pthread_mutex_unlock(&mutex);

            i++;
            if(j > 0){
                //create a new worker thread to replace the exit thread.
                if(pthread_create(&work_tid[i], NULL, worker_thread, &client_socket) == -1){
                    printf("server: failed to open worker thread.\n");
                    exit(1);
                }

                //join the new worker thread.
                if(pthread_join(work_tid[i], NULL) == -1){
                    printf("server: failed to join worker thread.\n");
                    exit(1);
                }
            }
        }
    }
    //destroy the locks.
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void *worker_thread(void *args){
    int client_socket = *((int*)args), size = strlen(term);
    char client_message[length], log_message[length];
    pthread_t log_tid;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    //while the server is recieving input from the client.
	while(recv(client_socket, client_message, sizeof(client_message), 0) != -1){
        //send the message back to the client.
		if(strncmp(client_message, term, size) != 0){
            if(send(client_socket, client_message, sizeof(client_message), 0) == -1){
                printf("server: send failed.\n");
                exit(1);
            }
        }

        //check if message is the terminating character.
		else if(strncmp(client_message, term, size) == 0){
            //send an exit message to client.
            if(send(client_socket, "server: ECHO SERVICE COMPLETE.", 100, 0) == -1){
                printf("server: send failed.\n");
                exit(1);
            }
            //close the client socket.
            close(client_socket);

            //lock the worker thread.
            pthread_mutex_lock(&mutex);
            
            //add a worker to replace the exit thread and signal main thread.
            work++;

            //signal the main thread and unlock the worker thread.
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }

        //copy the client message to the log message.
        strcpy(log_message, client_message);

        //open the log thread.
        if(pthread_create(&log_tid, NULL, log_thread, &log_message) == -1){
            printf("server: failed to open log thread.\n");
            exit(1);
        }
        //reset the client message.
        bzero(client_message, length);
	}

    //if the client exits print client exit message and exit.
    printf("server: client exited.\n");
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return NULL;
}

void *log_thread(void *args){
    char *log_message = args;
    time_t cal_time = time(NULL);

    //open log file for storing log information.
    FILE *log = fopen("log.txt", "a");
    if(log == NULL){
        printf("server: log file failed.\n");
    }

    //print the contents of the client message with respected calendar time to the log file.
    fprintf(log, "%s %s", log_message, asctime(localtime(&cal_time)));

    //close the log file and exit.
    fclose(log);
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
                printf("server: please enter the message length.\n");
                exit(1);
            }

            //set the argument after flag to length.
            length = atoi(argv[i+1]);

            //check if the value is sufficient.
            if(length < 128 || length > 2048){
                printf("server: insufficient message length.\n");
                exit(1);
            }
            //increment length flag.
            l++;
        }

        //if argv contains "-p" flag.
        else if(strcmp(argv[i], "-p") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter the port number.\n");
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
            if(work < 1 || work > 100){
                printf("server: insufficient number of workers.\n");
                exit(1);
            }
            //increment work flag.
            w++;
        }

        //if argv contains "-b" flag.
        else if(strcmp(argv[i], "-b") == 0){
            //check is there is a value after flag.
            if(!argv[i+1]){
                printf("server: please enter a buffer size.\n");
                exit(1);
            }

            //set the argument after flag to buf.
            buf = atoi(argv[i+1]);

            //check if the value is sufficient.
            if(buf < 1 || buf > 100){
                printf("server: insufficient buffer size.\n");
                exit(1);
            }
            //increment buf flag.
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
            //increment term flag.
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
    if(w == 0 || b == 0 || t == 0){
        //print error message and exit.
        printf("server: enter workers (-w), buf (-b), and terminator (-t).\n");
        exit(1);
    }
}