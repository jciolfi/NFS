/*
 * client.c -- TCP Socket Client
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "client_handler.h"

int main(int argc, char *argv[]) {
    // Process the command line input.
    if (argc < 3 || argc > 4) {
        printf("Invalid command line input.\nUsage: ./fget [GET| INFO | PUT | MD | RM] [options]\n");
        return -1;
    }

    // Connect to the server using TCP sockets.
    char* server_ip_address = malloc(sizeof(char) * 30);
    char* server_port_string = malloc(sizeof(char) * 10);
    int socket_desc = connect_to_server(server_ip_address, server_port_string);
    if (socket_desc < 0) {
        // Connection failed.
        return -1;
    }

    // parse request, send it, handle responses.
    parse_fget(argc, argv, socket_desc);

    // free variables, close socket.
    free(server_ip_address);
    free(server_port_string);
    close(socket_desc);
    return 0;
}