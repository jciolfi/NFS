/*
*  client_handler.c / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#include "client_handler.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ---------- Methods not defined in client_handler.h ----------

/**
 * Send the given request to the server.
 * 
 * @param socket_desc the descriptor for the socket.
 * @param request the request to send.
 * @param request_size the size of the request.
*/
void fsend(int socket_desc, char* request, long request_size) {
    // send request, exit if failed.
    if (send(socket_desc, request, request_size, 0) < 0) {
        printf("Failed to send request to server.\n");
        exit(-1);
    }
}

/**
 * Receive a response from the server and deserialize it.
 * 
 * @param socket_desc the descriptor for the socket.
 * @param server_response the raw response from the server to write into.
 * @param resp the response to set after deserialization.
*/
void freceive(int socket_desc, char* server_response, response_t* resp) {
    // Receive and display the server's response.
    memset(server_response, '\0', SERVER_RESPONSE_SIZE);
    if (recv(socket_desc, server_response, SERVER_RESPONSE_SIZE, 0) < 0) {
        printf("Failed to receive the server's response: %s.\n", strerror(errno));
        exit(-1);
    }

    // Deserialize and print response.
    deserialize_response_t(server_response, resp);
    print_response_t(resp);
}

/**
 * Send a request to the server and print the response.
 * 
 * @param argv the command line arguments to fget.
 * @param request_length the length of the command line request.
 * @param socket_desc the descriptor for the socket.
*/
void send_request(char *argv[], int request_length, int socket_desc) {
    // send request.
    char server_request[request_length];
    sprintf(server_request, "%s %s", argv[1], argv[2]);
    fsend(socket_desc, server_request, request_length);

    // get response.
    char server_response[SERVER_RESPONSE_SIZE];
    response_t resp;
    freceive(socket_desc, server_response, &resp);
}

/**
 * Handle GET reponse from the server by saving the data to the local file.
 * 
 * @param file_path the path to the local file to save the data to.
 * @param data the data to write to the file.
 * @param data_size the size of the data.
*/
void save_GET(char* file_path, void* data, size_t data_size) {
    // open file to write to.
    FILE* file = fopen(file_path, "wb");
    if (file == NULL) {
        printf("Could not save to file %s\n.", file_path);
        return;
    }

    // write to file & close it.
    fwrite(data, sizeof(char), data_size, file);
    fclose(file);
}

/**
 * Send a GET request and save the file contents.
 * 
 * @param argc the argument count.
 * @param argv the arguments to fget.
 * @param request_length the length of the request to the command line.
 * @param socket_desc the descriptor for the socket with the server.
*/
void send_GET(int argc, char *argv[], int request_length, int socket_desc) { 
    // send request.
    char server_request[request_length];
    sprintf(server_request, "%s %s", argv[1], argv[2]);
    fsend(socket_desc, server_request, request_length);

    // get response.
    char server_response[SERVER_RESPONSE_SIZE];
    response_t resp;
    freceive(socket_desc, server_response, &resp);

    // server could not find file to get.
    if (resp.status != 200) {
        exit(1);
    }

    // handle based on whether remote file path is omitted or not.
    if (argc == 3) {
        // remote file/path is omitted - use local file path.
        save_GET(argv[2], resp.data, resp.data_size);
    } else if (argc == 4) {
        // remote file/path is not omitted.
        save_GET(argv[3], resp.data, resp.data_size);
    } else {
        printf("Too many arguments supplied to GET\n.");
        exit(1);
    }
}

/**
 * Parse the local file and send the PUT request.
 * 
 * @param argc the argument count.
 * @param argv the arguments to fget.
 * @param request_length the length of the request to the command line.
 * @param socket_desc the descriptor for the socket with the server.
*/
void send_PUT(int argc, char *argv[], int request_length, int socket_desc) {
    // prepare the request with the action and remote file location/path.
    char server_request_part_1[request_length];
    if (argc == 3) {
         // remote file/path is omitted - use local file path.
         sprintf(server_request_part_1, "%s %s", argv[1], argv[2]);
    } else if (argc == 4) {
         // remote file/path is not omitted.
         sprintf(server_request_part_1, "%s %s", argv[1], argv[3]);
    } else {
         printf("Too many arguments supplied to PUT\n.");
         exit(1);
    }

    // open the local file.
    FILE *file = fopen(argv[2], "r");
    if (file == NULL) {
        printf("Could not find PUT file: %s\n", argv[2]);
        exit(1);
    }

    // determine file size.
    if (fseek(file, 0, SEEK_END) < 0) {
        fclose(file);
        printf("Could not find size of file: %s", argv[1]);
        exit(1);
    }
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the file into a buffer.
    char file_buffer[file_size + 1];
    fread(file_buffer, file_size, 1, file);
    file_buffer[file_size] = '\0';

    // build the request with the operation, the remote file location/path, and the file content.
    char server_request[request_length + file_size + 2];
    sprintf(server_request, "%s %s", server_request_part_1, file_buffer);

    // send request.
    fsend(socket_desc, server_request, sizeof(server_request));

    // get response.
    char server_response[SERVER_RESPONSE_SIZE];
    response_t resp;
    freceive(socket_desc, server_response, &resp);
}

/**
 * Get the length of the command line request.
 * 
 * @param argc the argument count.
 * @param argv the command line arguments to fget.
*/
int get_request_length(int argc, char *argv[]) {
    // sum the length of all arguments.
    int request_length = 1;
    for (int i = 1; i < argc; ++i) {
        request_length += strlen(argv[i]) + 1;
    }
    return request_length;
}

// ---------- Methods defined in client_handler.h ----------

int connect_to_server(char* server_ip_address, char* server_port_string) {
    // Try to read server settings from file.
    FILE *file = fopen(CLIENT_CONNECTION_SETTINGS, "r");
    if (file == NULL) {
        printf("Could not find connection settings file: %s", CLIENT_CONNECTION_SETTINGS);
        return -1;
    }

    // Read the server port number.
    while (!feof(file)) {
        char character = fgetc(file);
        if (character == '\n') {
            break;
        } else {
            strncat(server_port_string, &character, 1);
        }
    }

    // Read the server IP address.
    while (!feof(file)) {
        char character = fgetc(file);
        if (character == '\n' | character == '\xff') {
            break;
        } else {
            strncat(server_ip_address, &character, 1);
        }
    }

    // Close the file.
    fclose(file);

    // Convert the server port number to an int.
    int server_port = atoi(server_port_string);

    // Create socket.
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    // Check for success.
    if (socket_desc < 0) {
        printf("Client socket creation failed.\n");
        return -1;
    }

    // Set port and IP.
    struct sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(server_port);
    server_info.sin_addr.s_addr = inet_addr(server_ip_address);

    // Connect to the server.
    if (connect(socket_desc, (struct sockaddr*)&server_info, sizeof(server_info)) < 0) {
        printf("Connection to server failed.\n");
        return -1;
    }

    return socket_desc;
}

void parse_fget(int argc, char *argv[], int socket_desc) {
    int request_length = get_request_length(argc, argv);

    // check the action (argv[1]) and delegate to the correct handler.
    if (strcmp(argv[1], "GET") == 0) {
        send_GET(argc, argv, request_length, socket_desc);
    } else if (strcmp(argv[1], "INFO") == 0) {
        if (argc > 3) {
            printf("Too many arguments supplied to INFO.");
            exit(-1);
        }
        send_request(argv, request_length, socket_desc);
    } else if (strcmp(argv[1], "MD") == 0) {
        if (argc > 3) {
            printf("Too many arguments supplied to MD.");
            exit(-1);
        }
        send_request(argv, request_length, socket_desc);
    } else if (strcmp(argv[1], "PUT") == 0) {
        send_PUT(argc, argv, request_length, socket_desc);
    } else if (strcmp(argv[1], "RM") == 0) {
        if (argc > 3) {
            printf("Too many arguments supplied to RM.");
            exit(-1);
        }
        send_request(argv, request_length, socket_desc);
    } else {
        printf("Unsupported action: '%s'\n", argv[1]);
    }
}