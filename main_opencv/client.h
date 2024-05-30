#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h> // To define struct sockaddr_in

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CLIENTS 2
#define MAP_ROW 5
#define MAP_COL 5

// Structures copied from server.h
typedef struct {
    int socket;
    struct sockaddr_in address;
    int row;
    int col;
    int score;
    int bomb;
} client_info;

enum Status {
    nothing, //0
    item, //1
    trap //2
};

typedef struct {
    enum Status status;
    int score;
} Item;

typedef struct {
    int row;
    int col;
    Item item;
} Node;

typedef struct {
    client_info players[MAX_CLIENTS];
    Node map[MAP_ROW][MAP_COL];
} DGIST;

enum Action {
    move, //0
    setBomb //1
};

typedef struct {
    int row;
    int col;
    enum Action action;
} ClientAction;

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
