server client: server.c client.c
	gcc -o server server.c -lpthread -Wall -Werror
	gcc -o client client.c -Wall -Werror