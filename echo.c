#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

#define DEFAULT_LENGTH 4608
#define DEFAULT_PORT 256

int main(int argc, char **argv){
    int length, port;
    if(argc == 1){
        length = DEFAULT_LENGTH;
        port = DEFAULT_PORT;
    }
    else if(argc == 2){
        length = atoi(argv[1]);
        port = DEFAULT_PORT;
        if(port < 256 || port > 1024){
            printf("echo: insufficient port value.\n");
            exit(1);
        }
    }
    else if(argc == 3){
        length = atoi(argv[1]);
        port = atoi(argv[2]);
        if(length < 1024 || length > 9216){
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
    return 0;
}