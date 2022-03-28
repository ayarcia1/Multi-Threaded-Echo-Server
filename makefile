echo: server.c client.c
	gcc -o echo server.c client.c -lpthread -Wall -Werror