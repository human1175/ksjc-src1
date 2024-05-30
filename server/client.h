#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"

int connect_to_server(int port);
void send_client_action(int sock, ClientAction *action);
void receive_server_response(int sock);

#endif // CLIENT_H
