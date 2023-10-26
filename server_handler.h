#ifndef SERVER_HANDLER_H
#define SERVER_HANDLER_H

#define SERVER_CONNECTION_SETTINGS "config/connection_settings.txt"
#define DRIVE_CONFIG "config/drive_path_settings.txt"
#define MAX_DRIVE_PATH 128
#define SERVER_JOURNAL "journal/server_journal.txt"
#define SERVER_JOURNAL_TEMP "journal/server_journal_temp.txt"
#define CLIENT_REQUEST_SIZE 8192

#include <stdlib.h>
#include "common.h"

/**
 * Parse a request to the filesystem based on the actions: GET, INFO, MD, PUT, RM.
 * 
 * @param request the request string to parse.
 * @param resp the response to write to.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void parse_request(char* request, response_t* resp, char* drive1_path, char* drive2_path);

/**
 * Handle a request to get the contents of a file with the given file path.
 * 
 * @param file_path the path of the file to get.
 * @param resp the response to write to.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void handle_GET(char* file_path, response_t* resp, char* drive1_path, char* drive2_path);

/**
 * Handle a request to get the info of a file with the given file path.
 * 
 * @param file_path the path of the file to get.
 * @param resp the response to write to.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void handle_INFO(char* file_path, response_t* resp, char* drive1_path, char* drive2_path);

/**
 * Handle a request to make a directory at the given path.
 * 
 * @param dir_path the path of the directory to make.
 * @param resp the response to write to.
 * @param original_request the original request that is stored in the journal if the operation is not applied to both
 * replicas.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void handle_MD(char* dir_path, response_t* resp, char* original_request, char* drive1_path, char* drive2_path);

/**
 * Handle a request to create a file at the given path and put the given bytes in that file.
 * If the file already exists, it will be overwritten.
 * 
 * @param file_path the path of the file to upsert.
 * @param content the content to write to the file.
 * @param content_size the number of bytes to write to the file.
 * @param resp the response to write to.
 * @param original_request the original request that is stored in the journal if the operation is not applied to both
 * replicas.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void handle_PUT(char* file_path, void* content, long content_size, response_t* resp, char* original_request, char* drive1_path, char* drive2_path);

/**
 * Handle a request to remove a file or directory with the given path.
 * 
 * @param path the path to the file or directory to remove.
 * @param resp the response to write to.
 * @param original_request the original request that is stored in the journal if the operation is not applied to both
 * replicas.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void handle_RM(char* path, response_t* resp, char* original_request, char* drive1_path, char* drive2_path);

/**
 * Bind the server to the TCP socket described with the given port and IP in the settings file.
 * 
 * @param server_ip_address the allocated string to hold the server IP.
 * @param server_port_string the allocated string to hold the server port.
 * @return the socket description.
 */
int bind_server_socket(char* server_ip_address, char* server_port_string);

/**
 * Set the drive paths from config.
 * 
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
*/
void set_drive_paths(char* drive1_path, char* drive2_path);

#endif