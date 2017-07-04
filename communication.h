#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

void send_message(int socket, char *message);
void read_message(int socket, char *res_buf, int size);

#endif
