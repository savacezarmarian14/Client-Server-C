#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util.h" // ASSERT

#define MAX_CLIENTS 1024

int handle_new_connection(int listenfd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_len;
    int client_socket;

    // Cererile de conectare la server sunt puse intr-o coada. Se accepta prima cerere lansata
    client_socket = accept(listenfd, (struct sockaddr *) &client_address, &client_address_len);
    
    if (client_socket < 0) {
        printf("Error connecting new client to server.\n");
    } else {
    printf("New Client joined on socket %d, from port %d, with IP %s\n", 
		client_socket, ntohs(client_address.sin_port), inet_ntoa(client_address.sin_addr));
    }

    return client_socket;
}

void handle_server_commnad(fd_set fds, int maxfd)
{  
    char buffer[1024];
    int ret;

    // Citesc din bufferul de la STDIN pentru a prelua comanda (singura comanda este exit)
    ret = read(STDIN, buffer, 1024);
    buffer[ret-1] = 0;

    if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "EXIT") == 0) {
        for (int i = 3; i < maxfd + 1; ++i) {
            if (FD_ISSET(i, &fds)) {
                printf("Closing %d\n", i);
                shutdown(i, SHUT_RDWR);
                close(i);
            }
        }
        exit(0);
    }
   
}

void recive_message_from_client(int sockfd)
{
    char message[1024];
    memset(message, 0, 1024);
    int ret;

    ret = recv(sockfd, message, 1024, 0); 
    printf("Message from client [%d]: %s\n", sockfd, message);
}

int main(int argc, char *argv[])
{
    int listenfd;
    struct sockaddr_in serv_address;
    socklen_t serv_address_len = sizeof(struct sockaddr_in);
    int fdmax, ret;


    // Initializam socket ul pentru ascultarea noilor conexiuni
    // int socket(int domain, int type, int protocol)
    // domain: AF_INET -> Familia de adrese (IPV4). Aceste adrese
    // sunt pe 4 bytes. Ex: 255.255.255.255
    // type: SOCK_STREAM -> Protocolul folosit (TCP: Transfer control protocol)
    // protocol: "Normally only a single protocol exists to support a
    //           particular socket type within a given protocol family, in which
    //           case protocol can be specified as 0."

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    // Initializam serverul cu IP-ul si PORT-ul sau (argv[1]:IP, argv[2]:PORT)
        // Initializam structura sockaddr_in cu FAMILIA DE ADRESE
    serv_address.sin_family = AF_INET;
        // Initializam structura sockaddr_in cu IP-ul serverului
        // ATON: ASCII to NETWORK
    ret = inet_aton(argv[1], &serv_address.sin_addr);
    ASSERT(ret != 0, "Invalid server address");
        // Initializam structura sockaddr_in cu PORT-ul serverului
    serv_address.sin_port = htons(atoi(argv[2]));

    // Legam adresa IP si PORT-ul abia setat mai sus cu socket-ul
    // prin care ascultam noi conexiuni
    ret = bind(listenfd, (struct sockaddr *) &serv_address, serv_address_len);
    ASSERT(ret == 0, "Error bind");

    // Setam socket-ul "listenfd" sa asculte noi conexiuni
    ret = listen(listenfd, MAX_CLIENTS);
    
    // Initializam doua seturi
        // Unul permanent care va retine toti socketii (listenfd + clients)
        // Unul temporar in care dupa apelul SELECT vor ramane doar socketii unde s au facut modificari
    fd_set permanent_fds; FD_ZERO(&permanent_fds);
    fd_set temporar_fds;  FD_ZERO(&temporar_fds);

    // Adaugam socket-ul pentru listen in setul permanent pentru a fi tratat
    FD_SET(listenfd, &permanent_fds); fdmax = listenfd;
    // Adaugam STDIN pentru a putea da comenzi serverului
    FD_SET(0, &permanent_fds);

    // =================== SERVER LOOP ================== //
    // Serverul va face acest loop permanent. TODO: De ce //
    
    // Pentru a face un server asincron trebuie sa ne imaginam urmatoarele scenarii
    //  1. Un client nou vrea sa se conecteze la server
    //  2. Un client deja conectat vrea sa trimita un mesaj
    // Serverul nu stie care dintre aceste scenarii urmeaza (asta inseamna asincron)
    while(1) {
        // Retinem setul PERMANENT in setul TEMPORAR pentru a vedea pe ce socket s-a efectuat o modificare
        temporar_fds = permanent_fds;
        // Apelam functia select pentru a detecta modificarile
        // Dupa efectuarea apelului, in temporar_fds vor ramane doar socketii pe care s au trimis date
        ret = select(fdmax + 1, &temporar_fds, NULL, NULL, NULL);
        
        // In momentul actual in temporar_fds au ramas doar socketii pe care s au efectuat o operatie
        // Iteram prin toti socketii cunoscuti pentru a vedea care din ei au ramas in set
        for (int i = 0; i < fdmax + 1; ++i) {
            // Verific daca file descriptorul i a ramas in set
            if (FD_ISSET(i, &temporar_fds) != 0) {
                // Comanda pentru server
                if (i == STDIN) {
                    handle_server_commnad(permanent_fds, fdmax);
                }
                // 1. Un client nou vrea sa se conecteze la server
                if (i == listenfd) {
                    // Acceptam noua conexiune
                    int new_client_socket = handle_new_connection(listenfd);
                    // Daca este o conexiune valida, adaugam noul socket in setul PERMANENT
                    if (new_client_socket >= 0) {
                        FD_SET(new_client_socket, &permanent_fds);
                        fdmax += 1; // Vom discuta asta la laborator
                    }
                }
                // 2. Un client deja conectat vrea sa trimita un mesaj
                else if (i != listenfd && i != STDIN) {
                    printf("Message\n");
                    recive_message_from_client(i);
                }
            }
        }
    }




    return 0;
}
