#ifndef CLIENT_H
#define CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

// Function to start the client and connect to the server
void start_client(const char *server_ip, int server_port);

// Function to send QR result to the server
void send_qr_result_to_server(const char* qr_result);

// Thread function to receive data from the server
void* receive_data_from_server(void* arg);

// Function to print the DGIST structure
void print_dgist(DGIST* dgist);

#ifdef __cplusplus
}
#endif

#endif // CLIENT_H
