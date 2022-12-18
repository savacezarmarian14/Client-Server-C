#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util.h"


void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

void send_message_to_server(int sockfd)
{
    char message[1024];
    int ret, message_len;
    memset(message, 0, 1024);

    printf("Input: ");
    fflush(stdout); // TODO: De ce apelez aceasta functie
    
    // Citim input-ul de la tastatura
    ret = read(STDIN, message, 1024); // STDIN definit un util.h (ghiciti ce este ? :) )
    message[ret-1] = 0; // Deoarece citim si \n SI NU VREM ASTA
    message_len = strlen(message);

    // Trimitem mesajul catre server
    ret = send(sockfd, message, message_len, 0);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in serv_addr;

    int ret;

	if (argc < 3) {
		usage(argv[0]);
	}

    // Initializam socket-ul CLIENTULUI (IPV4, TCP)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(sockfd >= 0, "Initialize socket");
    // Initializam datele pentru server pentru a ne putea conecta
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	ret = inet_aton(argv[1], &serv_addr.sin_addr);
    ASSERT(ret != 0, "Setting server address");

    // Trimitem cerere de conectare la server (3 WAY HANDSHAKE <- google it) 
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

	while (1) {
        /* NOTHING */
        send_message_to_server(sockfd);
	}

	close(sockfd);

	return 0;
}
