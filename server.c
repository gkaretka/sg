#include "server.h"
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include "constants.h"

static int port = 8989;

struct sockaddr_in server_addr, client_addr;
int sock, c_sock;

void my_bind(void);

int *init_server(void) {
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    my_bind();

    listen(sock , 5);

    printf("server listening on %s:%d \n", 
            inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    int c = sizeof(struct sockaddr_in);
    c_sock = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&c);
    
    if (c_sock < 0)
    {
        perror("accept failed");
    }

    return &c_sock;
}

void my_bind(void) { 
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        port++;
        server_addr.sin_port = htons(port);
        my_bind();
    }
}
