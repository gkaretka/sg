#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

void send_message(int socket, char *message);
void read_message(int socket, char *res_buf, int size);

#endif
