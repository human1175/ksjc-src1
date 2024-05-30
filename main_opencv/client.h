#ifndef CLIENT_H
#define CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

// Function to start the client and connect to the server
void start_client(const char *server_ip, int server_port);

#ifdef __cplusplus
}
#endif

#endif // CLIENT_H
