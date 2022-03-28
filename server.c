#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
void echo(int connfd);
void *thread(void *vargp);

#define DEFAULT_LENGTH 1024
#define DEFAULT_PORT 256
#define BUFFER_SIZE 1024
#define NUM_WORKER 10
#define EXIT_CHAR "x"
#define SA struct sockaddr

int main(int argc, char **argv){
    int i, length, port, listenfd, *connfd;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid, wthread[NUM_WORKER];

    if(argc == 1){
        length = DEFAULT_LENGTH;
        port = DEFAULT_PORT;
    }

    else if(argc == 2){
        length = atoi(argv[1]);
        port = DEFAULT_PORT;
        if(length < 512 || length > 2048){
            printf("echo: insufficient length value.\n");
            exit(1);
        }
    }

    else if(argc == 3){
        length = atoi(argv[1]);
        port = atoi(argv[2]);

        if(length < 512 || length > 2048){
            printf("echo: insufficient length value.\n");
            exit(1);
        }
        
        if(port < 128 || port > 512){
            printf("echo: insufficient port value.\n");
            exit(1);
        }
    }

    else if(argc > 3){
        printf("echo: error, too many parameters.\n");
        exit(1);
    }

    for(i=0; i<NUM_WORKER; i++){
        pthread_create(&wthread[i], NULL, thread, NULL);
        if(pthread_create(&wthread[i], NULL, thread, NULL) == -1){
            printf("echo: failed to open thread\n");
            exit(1);
        }
    }

    while(1){
        connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, (SA*) &clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfd);
    }
    return 0;
}

void echo(int connfd){
    size_t n;
    char buf[BUFFER_SIZE];

}

void *thread(void *vargp){
    int connfd = *((int*)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    echo(connfd);
    return NULL;
}
