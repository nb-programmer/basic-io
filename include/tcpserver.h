#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <netinet/in.h>

struct _tcp_server;
typedef int(*tcpserver_handler)(int cli_sock, struct _tcp_server *tcpsv);

typedef struct _tcp_server {
    /* Public */
    //Port to listen to
    unsigned int listen_port;
    //Which function to call when a new client connects
    tcpserver_handler client_handler;

    /* Private */
    //Listening socket file descriptor
    int listen_sock;
    //Incoming Client socket file descriptor
    int cli_sock;
    //Addresses for listening socket and incoming connections
    struct sockaddr_in servsock_addr, clisock_addr;
    int cli_addr_len;
} tcp_server;

//Public functions
int tcpserver_create(tcp_server *tcpsv);
int tcpserver_startlistening(tcp_server *tcpsv);
void tcpserver_close(tcp_server *tcpsv);

#endif