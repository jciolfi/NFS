/*
*  client_handler.h / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#define CLIENT_CONNECTION_SETTINGS "config/connection_settings.txt"
#define SERVER_RESPONSE_SIZE 8192

#include "common.h"

/**
 * Connect the client to the server using TCP sockets and a settings file.
 * 
 * @param server_ip_address the allocated string to hold the server IP.
 * @param server_port_string the allocated string to hold the server port.
 * @return the socket description.
 */
int connect_to_server(char* server_ip_address, char* server_port_string);

/**
 * Parse a fget request to send and handle the corresponding request/response.
 * 
 * @param argc the argument count.
 * @param argv the arguments to fget.
 * @param socket_desc the descriptor for the socket to send/recv to/from.
*/
void parse_fget(int argc, char *argv[], int socket_desc);


#endif