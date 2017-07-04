#include "communication.h"
#include "constants.h"

extern const int buf_size;

void send_message(int socket, char *message) {
    char recv_msg[buf_size];

    if (send(socket, message, strlen(message), 0) < 0) {
        perror("failed to send message");
    }
}

void read_message(int socket, char *res_buf, int size) {
    char buf[size];
    
    recv(socket, buf, size, 0);
    
    memcpy(res_buf, buf, sizeof(char) * size);
}
