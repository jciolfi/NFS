/*
*  server_main.c / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "server_handler.h"

#define MAX_CLIENTS 8


// server descriptor and IP addresses to close/free on shutdown.
int server_desc;
char* server_ip_address;
char* server_port_string;

// drive paths for the server
char drive1_path[MAX_DRIVE_PATH];
char drive2_path[MAX_DRIVE_PATH];

// threads with lock to maintain file consistency.
static int client_sockets[MAX_CLIENTS];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// optional delay for client handling to emphasize multithreading.
static int client_delay = 0;

/**
 * Handle given, safely exit by trapping SIGINT signal. 
 * 
 * @param sig the signal to handle.
*/
void handle_signal(int sig) {
    if (sig == SIGINT) {
        // close open ports and free other variables.
        for (int i = 0; i < MAX_CLIENTS; i++) {
            close(client_sockets[i]);
        }
        close(server_desc);
        free(server_ip_address);
        free(server_port_string);
        pthread_mutex_destroy(&lock);
        printf("\n\nExited safely.\n");
        exit(0);
    }
}

/**
 * Handle a client request in a thread.
 * 
 * @param data the client index.
 * @return NULL always.
*/
void* handle_client(void* data) {
    // get client socket.
    int client_idx = *(int*) data;
    int client_socket = client_sockets[client_idx];

    // Receive a request from the client.
    char client_request[CLIENT_REQUEST_SIZE];
    memset(client_request, '\0', sizeof(client_request));
    int nbytes = recv(client_socket, client_request, CLIENT_REQUEST_SIZE, 0);
    if (nbytes == 0) {
        // indicates client socket was closed.
        printf("Client connection for socket %d was closed. Exiting...\n", client_socket);
        close(client_socket);
        return NULL;
    } else if (nbytes < 0) {
        printf("Server error while receiving client request: %s.\n", strerror(errno));
        close(client_socket);
        return NULL;
    }

    // sleep to emphasize multithreading.
    usleep(client_delay * 1000000);

    // Process the client request.
    response_t resp;
    pthread_mutex_lock(&lock);
    parse_request(client_request, &resp, drive1_path, drive2_path);
    pthread_mutex_unlock(&lock);

    char* resp_serialized = serialize_response_t(&resp);
    unsigned long* size_ptr = (unsigned long*) resp_serialized;
    unsigned long size = *size_ptr;

    if (send(client_socket, resp_serialized, size, 0) < 0) {
        printf("Failed to send response to client.\n");
        free(resp.data);
        free(resp_serialized);
        close(client_socket);
        return NULL;
    }

    free(resp.data);
    free(resp_serialized);
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    // parse out client delay if specified. Default is none.
    if (argc == 2) {
        client_delay = atoi(argv[1]);
        if (errno == ERANGE || (client_delay == 0 && argv[1][0] != '0')) {
            printf("Client delay must be specified as an integer. The delay unit is seconds.\n");
            return 1;
        }
    } else if (argc > 2) {
        printf("Too many arguments supplied to fserver.\n");
        return 1;
    }

    // establish paths for drives from config.
    set_drive_paths(drive1_path, drive2_path);

    // Prepare for TCP socket connection with the client.
    server_ip_address = malloc(sizeof(char) * 30);
    server_port_string = malloc(sizeof(char) * 10);
    server_desc = bind_server_socket(server_ip_address, server_port_string);
    if (server_desc < 0) {
        free(server_ip_address);
        free(server_port_string);
        printf("Could not bind to server socket: %s\n", strerror(errno));
        return -1;
    }

    // Trap SIGINT and close open ports.
    signal(SIGINT, handle_signal);

    // Listen for clients.
    if (listen(server_desc, MAX_CLIENTS) < 0) {
        free(server_ip_address);
        free(server_port_string);
        close(server_desc);
        printf("Server error while listening.\n");
        return -1;
    }

    // setup multithreading.
    pthread_t threads[MAX_CLIENTS];
    int thread_idxs[MAX_CLIENTS];
    int thread_idx = 0;

    while (1) {
        // Accept a client connection.
        struct sockaddr_in client_info;
        socklen_t client_size = sizeof(client_info);
        int client_socket = accept(server_desc, (struct sockaddr*)&client_info, &client_size);
        if (client_socket < 0) {
            printf("Server error while accepting the client connection.\n");
            continue;
        }
        client_sockets[thread_idx] = client_socket;

        printf("Client connected with port = %i and IP = %s\n",
            ntohs(client_info.sin_port), inet_ntoa(client_info.sin_addr));

        thread_idxs[thread_idx] = thread_idx;
        if (pthread_create(&threads[thread_idx], NULL, handle_client, (void*) &thread_idxs[thread_idx]) < 0) {
            printf("Unable to create thread for client with port = %i and IP = %s\n",
                ntohs(client_info.sin_port), inet_ntoa(client_info.sin_addr));
            close(client_socket);
            continue;
        }

        thread_idx = (thread_idx + 1) % MAX_CLIENTS;
    }

    // Close the client and server sockets.
    free(server_ip_address);
    free(server_port_string);
    close(server_desc);
    return 0;
}