
#include "tcpserver.h"

//Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Linux file IO libraries
#include <fcntl.h>
#include <unistd.h>

//Linux Socket libraries
#include <sys/socket.h>

//Creates a listening TCP socket object
int tcpserver_create(tcp_server *tcpsv) {
    //Create a new TCP socket
    tcpsv->listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    tcpsv->cli_addr_len = sizeof(tcpsv->clisock_addr);
    if (tcpsv->listen_sock == -1) {
        fprintf(stderr, "Failed to open listening socket\n");
        return 1;
    }

    //Set the address to listen to, which is any ip address
    memset(&(tcpsv->servsock_addr), 0, sizeof(tcpsv->servsock_addr));
    tcpsv->servsock_addr.sin_family = AF_INET;
    tcpsv->servsock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    tcpsv->servsock_addr.sin_port = htons(tcpsv->listen_port);

    //Enable the SO_REUSEADDR option
    int enable = 1;
    if (setsockopt(tcpsv->listen_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "Failed to set the SO_REUSEADDR socket option\n");
        return 1;
    }

    return 0;
}

//Enables listening mode on the socket
int tcpserver_startlistening(tcp_server *tcpsv) {
    //Bind listening socket to above IP/port
    if (bind(tcpsv->listen_sock, (struct sockaddr *)&(tcpsv->servsock_addr), sizeof(tcpsv->servsock_addr)) != 0) {
        fprintf(stderr, "Failed to bind to port %d. The port may be in use by another application.\n", tcpsv->listen_port);
        return 1;
    }

    //Listen for incoming connections with 10 connection backlogs
    if (listen(tcpsv->listen_sock, 10) != 0) {
        fprintf(stderr, "Failed to make listening socket on port %d\n", tcpsv->listen_port);
        return 1;
    }

    printf("Started listening on port %d...\n", tcpsv->listen_port);
    while (1) {
        tcpsv->cli_sock = accept(tcpsv->listen_sock, (struct sockaddr *)&(tcpsv->clisock_addr), &(tcpsv->cli_addr_len));
        if (tcpsv->cli_sock < 0) {
            fprintf(stderr, "Connection to client was not accepted. Resuming...\n");
            continue;
        }

        //TODO: Handle nullptr handler function with stub function
        if ((tcpsv->client_handler)(tcpsv->cli_sock, tcpsv) != 0) {
            fprintf(stderr, "Failed to handle TCP socket request\n");
            close(tcpsv->cli_sock);
        }
    }

    return 0;
}

//Destroy the listening socket
void tcpserver_close(tcp_server *tcpsv) {
    close(tcpsv->listen_sock);
}
