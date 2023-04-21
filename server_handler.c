/*
*  server_handler.c / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mount.h>
#include "server_handler.h"

/**
 * Open a file and return the file descriptor. Try both drives for success.
 * 
 * @param file_path the path to the file.
 * @param drive1_path the drive 1 path.
 * @param drive2_path the drive 2 path.
 * @return a pointer to the file descriptor for the file.
*/
FILE* open_file(char* file_path, char* drive1_path, char* drive2_path) {
    // open file on drive 1 (the main drive).
    char full_path[strlen(file_path) + strlen(drive1_path) + 1];
    sprintf(full_path, "%s/%s", drive1_path, file_path);
    FILE* file = fopen(full_path, "r");
    if (file == NULL) {
        sprintf(full_path, "%s/%s", drive2_path, file_path);
        file = fopen(full_path, "r");
    }

    return file;
}

/**
 * Create directory with the given path on the given drive. 
 * 
 * @param dir_path the path for the new directory.
 * @param drive the drive where the directory will be created.
 * @return true if successfully created, false otherwise.
*/
bool handle_MD_drive(char* dir_path, char* drive) {
    char full_dir_path[strlen(dir_path) + strlen(drive) + 1];
    sprintf(full_dir_path, "%s/%s", drive, dir_path);
    return mkdir(full_dir_path, 0755) == 0;
}

/**
 * Handle put request for the given drive.
 * 
 * @param file_path the path of the file to upsert.
 * @param content the content to write to the file.
 * @param content_size the number of bytes to write to the file.
 * @param drive the drive to put the file into.
 * @return true if successful, false otherwise.
*/
bool handle_PUT_drive(char* file_path, void* content, long content_size, char* drive) {
    // try to create file.
    char full_file_path[strlen(file_path) + strlen(drive) + 1];
    sprintf(full_file_path, "%s/%s", drive, file_path);
    FILE* file = fopen(full_file_path, "wb");
    if (file == NULL) {
        return false;
    }

    // write to file and close it.
    if (content != NULL && content_size > 0) {
        fwrite(content, content_size, 1, file);
    }
    fclose(file);

    return true;
}

/**
 * Handle remove item request for the given drive.
 * 
 * @param path the path to the file or directory to remove.
 * @param drive the drive to remove the file from.
 * @return true if remove was successful, false otherwise.
*/
bool handle_RM_drive(char* path, char* drive) {
    char full_path[strlen(path) + strlen(drive) + 1];
    sprintf(full_path, "%s/%s", drive, path);

    return remove(full_path) == 0;
}

/**
 * Checks both drives to see if they have the space required available.
 *
 * @param space_required - the space required in bytes.
 * @param drive_path - the path to the drive.
 * @return - true if the drive has the necessary space available and false if not.
 */
bool check_drive_free_space(uint64_t space_required, char* drive_path) {
    struct statfs drive_stats;
    if (statfs(drive_path, &drive_stats) == -1) {
        // Drive is not connected, but the request can be logged in the journal.
        return true;
    }
    uint32_t block_size = drive_stats.f_bsize;
    uint64_t free_blocks = drive_stats.f_bavail;
    uint64_t free_bytes = block_size * free_blocks;

    // Check that the drive has the required space available.
    return free_bytes >= space_required;
}

/**
 * Create a journal entry for a PUT, RM, or MD request with a given drive index.
 *
 * @param request the request string received from the client.
 * @param drive_index the index of the drive that the operation needs to be performed on.
 */
void create_journal_entry(char* request, int drive_index) {
    // Open journal file in append mode.
    FILE* file = fopen(SERVER_JOURNAL, "a");
    if (file == NULL) {
        return;
    }

    // Combine the request string with the index of the drive.
    int buffer_size = strlen(request) + 5;
    char buffer[buffer_size];
    sprintf(buffer, "%d%s\r\n\r\n", drive_index, request);

    // write to file and close it.
    if (request != NULL) {
        fwrite(buffer, buffer_size, 1, file);
    }
    fclose(file);
}

/**
 * Parse a request from the journal and apply it to a single drive. It is assumed that the request is correctly
 * formatted because it was already validated before it was recorded in the journal.
 *
 * @param request the request string to parse.
 * @param drive_index the drive to apply the request to.
 * @return true if the operation was applied and false if not.
 */
bool parse_request_single_drive(char* request, int drive_index, char* drive1_path, char* drive2_path) {
    // set the drive path based on the drive index.
    char* drive_path = NULL;
    if (drive_index == 1) {
        drive_path = drive1_path;
    } else if (drive_index == 2) {
        drive_path = drive2_path;
    } else {
        // invalid drive index.
        return false;
    }

    // get action of the request.
    char action[8];
    unsigned long i;
    for (i = 0; i < strlen(request); i++) {
        // parse request until a space is seen.
        if (request[i] != ' ') {
            action[i] = request[i];
        } else {
            action[i] = '\0';
            break;
        }
    }

    // the part of the request excluding the action.
    char request_slice[strlen(request) - i];
    memcpy(request_slice, &request[i + 1], strlen(request) - i);

    // determine the action (MD, RM, or PUT) and appropriate handler.
    if (strcmp(action, "MD") == 0) {
        return handle_MD_drive(request_slice, drive_path);
    } else if (strcmp(action, "PUT") == 0) {
        // extract file path and file data.
        char put_path[strlen(request_slice) + 1];
        unsigned long j;
        for (j = 0; j < strlen(request_slice); j++) {
            if (request_slice[j] != ' ') {
                put_path[j] = request_slice[j];
            } else {
                put_path[j] = '\0';
                break;
            }
        }

        // when no data to write to the file.
        if (j >= strlen(request_slice)) {
            put_path[strlen(request_slice)] = '\0';
            return handle_PUT_drive(put_path, NULL, 0, drive_path);
        }

        // when data to write to the file.
        char put_data[strlen(request_slice) - j];
        memcpy(put_data, &request_slice[j + 1], strlen(request_slice) - j);

        // check if the drive has enough space.
        if (!check_drive_free_space(strlen(request_slice) - j, drive_path)) {
            return false;
        }

        return handle_PUT_drive(put_path, put_data, sizeof(put_data), drive_path);
    } else if (strcmp(action, "RM") == 0) {
        return handle_RM_drive(request_slice, drive_path);
    } else {
        // Unsupported action.
        return false;
    }
}

/**
 * Attempts to perform all operations remaining in the journal of unfinished operations.
 *
 * @return true if all operations have been applied or false if some remain.
 */
bool process_journal_entries(char* drive1_path, char* drive2_path) {
    // Open the journal file.
    FILE* journal = fopen(SERVER_JOURNAL, "r");
    if (journal == NULL) {
        return false;
    }

    // Use fast and slow pointers to maintain the location of the last completed operation and the current operation.
    long int last_complete_operation = ftell(journal);

    // Apply the journal operations.
    while (!feof(journal)) {
        // Read the drive index from the journal for this line.
        char drive_index = fgetc(journal);
        int drive_index_int = drive_index - '0';

        // Read the request from the journal for this line.
        char request[CLIENT_REQUEST_SIZE];
        int request_length = 0;
        while (!feof(journal)) {
            char character = fgetc(journal);
            strncat(request, &character, 1);
            request_length++;

            // Check for "\r\n\r\n" to indicate end of entry.
            if (request_length > 4) {
                char* comparison_ptr = request + request_length - 4;
                if (strncmp(comparison_ptr, "\r\n\r\n", 4) == 0) {
                    // Clear "\r\n\r\n" from the request buffer.
                    request[request_length - 4] = '\0';
                    break;
                }
            }
        }

        // Try to apply the operation to the correct drive and break if the operation fails.
        if (!parse_request_single_drive(request, drive_index_int, drive1_path, drive2_path)) {
            request[0] = '\0';
            break;
        }

        // Clear the request buffer and update the location of the last completed operation.
        request[0] = '\0';
        last_complete_operation = ftell(journal);
    }

    // Set the location of the file pointer to the last complete operation.
    fseek(journal, last_complete_operation, SEEK_SET);

    // Clear the journal file or write remaining operations.
    if (!feof(journal)) {
        // Some operations are remaining.

        // Copy the remaining operations to a temporary file.
        FILE* new_journal = fopen(SERVER_JOURNAL_TEMP, "w");
        while (!feof(journal)) {
            char character = fgetc(journal);
            // Check for end of file character.
            if (character != EOF) {
                fputc(character, new_journal);
            }
        }

        // Delete the original journal file.
        fclose(journal);
        fclose(new_journal);
        remove(SERVER_JOURNAL);

        // Rename the temporary file to the original journal file.
        if (rename(SERVER_JOURNAL_TEMP, SERVER_JOURNAL) != 0) {
            // Error renaming the file.
            return false;
        }

        // Need to determine if the remaining operations all apply to a single drive.
        bool drive1_request_present = false;
        bool drive2_request_present = false;

        // Iterate through the remaining operations.
        FILE* journal_remaining = fopen(SERVER_JOURNAL, "r");
        while (!feof(journal_remaining)) {
            // Read the drive index from the journal for this line.
            char drive_index = fgetc(journal_remaining);
            int drive_index_int = atoi(&drive_index);

            // Read the request from the journal for this line.
            // TODO: update to match above
            char request[CLIENT_REQUEST_SIZE];
            int request_length = 0;
            while (!feof(journal)) {
                char character = fgetc(journal);
                strncat(request, &character, 1);
                request_length++;

                // Check for "\r\n\r\n" to indicate end of entry.
                if (request_length > 4) {
                    char *comparison_ptr = request + request_length - 4;
                    if (strncmp(comparison_ptr, "\r\n\r\n", 4) == 0) {
                        // Clear "\r\n\r\n" from the request buffer.
                        request[request_length - 4] = '\0';
                        break;
                    }
                }
            }

            // Record what drive this operation applies to.
            if (drive_index_int == 1) {
                drive1_request_present = true;
            } else if (drive_index_int == 2) {
                drive2_request_present = true;
            }

            // Clear the request buffer.
            request[0] = '\0';
        }

        // Close the file
        fclose(journal_remaining);

        // Check if all the remaining operations apply to a single drive.
        if (drive1_request_present && drive2_request_present) {
            // Operations remain for both drives, don't allow new operations.
            return false;
        } else {
            // All the remaining operations only apply to a single drive, allow new operations on the other drive.
            return true;
        }
    } else {
        // No operations are remaining, clear the journal.
        fclose(journal);
        FILE* journal_cleared = fopen(SERVER_JOURNAL, "w");
        fclose(journal_cleared);
        return true;
    }
}

/**
 * Set the drive paths based on the file defined by DRIVE_CONFIG.
 *
 * @param drive1_path - the path for drive 1.
 * @param drive2_path - the path for drive 2.
 */
void set_drive_paths(char* drive1_path, char* drive2_path) {
    FILE* file = fopen(DRIVE_CONFIG, "r");
    if (file == NULL) {
        printf("Could not open config for drive paths.\n");
        exit(1);
    }

    // get file size.
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // set each drive path var.
    if (fgets(drive1_path, file_size, file) == NULL) {
        puts("Could not get path for drive 1.");
        exit(1);
    } else {
        // set last char to null terminating instead of newline.
        drive1_path[strlen(drive1_path) - 1] = '\0';
    }

    if (fgets(drive2_path, file_size, file) == NULL) {
        puts("Could not get path for drive 2.");
        exit(1);
    }

    fclose(file);
}

// ---------- Methods defined in server_handler.h ----------

void parse_request(char* request, response_t* resp, char* drive1_path, char* drive2_path) {
    // Process journal entries so that the drive replicas are in a consistent state.
    if (!process_journal_entries(drive1_path, drive2_path)) {
        printf("Unable to clear journal entries to ensure consistent server state\n");
        build_response_t(resp, 404, "Unable to clear journal entries", NULL, 0);
        return;
    }

    // get action of the request.
    action_t action = get_action(request);

    // determine appropriate handler, extract path for request.
    switch (action) {
        case GET: {
            int action_len = strlen("GET");
            char file_path[strlen(request) - action_len];
            memcpy(file_path, &request[action_len + 1], strlen(request) - action_len);
            handle_GET(file_path, resp, drive1_path, drive2_path);
            break;
        }
        case INFO: {
            int action_len = strlen("INFO");
            char file_path[strlen(request) - action_len];
            memcpy(file_path, &request[action_len + 1], strlen(request) - action_len);
            handle_INFO(file_path, resp, drive1_path, drive2_path);
            break;
        }
        case MD: {
            int action_len = strlen("MD");
            char dir_path[strlen(request) - action_len];
            memcpy(dir_path, &request[action_len + 1], strlen(request) - action_len);
            handle_MD(dir_path, resp, request, drive1_path, drive2_path);
            break;
        }
        case PUT: {
            int action_len = strlen("PUT");

            // extract path variable.
            char put_path[strlen(request) - action_len];
            unsigned long i;
            for (i = action_len + 1; i < strlen(request); i++) {
                if (request[i] != ' ') {
                    put_path[i - (action_len + 1)] = request[i];
                } else {
                    put_path[i - (action_len + 1)] = '\0';
                    break;
                }
            }

            // extract data portion of request.
            if (i >= strlen(request)) {
                put_path[strlen(request) - action_len - 1] = '\0';
                handle_PUT(put_path, NULL, 0, resp, request, drive1_path, drive2_path);
            } else {
                long data_size = strlen(request) - action_len - i + 2;
                void* put_data = malloc(data_size + 1);
                memcpy(put_data, &request[i + 1], data_size);
                handle_PUT(put_path, put_data, data_size, resp, request, drive1_path, drive2_path);
                free(put_data);
            }
            break;
        }
        case RM: {
            int action_len = strlen("RM");
            char file_path[strlen(request) - action_len];
            memcpy(file_path, &request[action_len + 1], strlen(request) - action_len);
            handle_RM(file_path, resp, request, drive1_path, drive2_path);
            break;
        }
        default: {
            build_response_t(resp, 405, "Unsupported action", NULL, 0);
        }
    }
}

void handle_GET(char* file_path, response_t* resp, char* drive1_path, char* drive2_path) {
    // open file on drive 1 (the main drive).
    FILE* file = open_file(file_path, drive1_path, drive2_path);
    if (file == NULL) {
        build_response_t(resp, 404, "Could not find file to GET.", NULL, 0);
        return;
    }

    // get file length.
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read file contents.
    size_t data_size = file_size + 1;
    char* data = malloc(data_size);
    fread(data, 1, file_size, file);
    data[file_size] = '\0';
    
    // close file and write to response.
    fclose(file);
    build_response_t(resp, 200, "Successfully found file to GET.", data, data_size);
}

void handle_INFO(char* file_path, response_t* resp, char* drive1_path, char* drive2_path) {
    // open file on drive 1 (the main drive).
    FILE* file = open_file(file_path, drive1_path, drive2_path);
    if (file == NULL) {
        build_response_t(resp, 404, "Could not find INFO for file.", NULL, 0);
        return;
    }

    // get file length.
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // get file owner, permissions
    struct stat file_stat;
    int owner_id;
    time_t last_modified;
    mode_t permissions;
    
    if (stat(file_path, &file_stat) == 0) {
        size_t data_size = sizeof(owner_id) + sizeof(last_modified) + sizeof(permissions) + sizeof(file_size) + 64;
        char* data = malloc(data_size);
        owner_id = file_stat.st_uid;
        last_modified = file_stat.st_mtime;
        permissions = file_stat.st_mode;
        sprintf(data, "size=%ld, owner UID=%d, last modified=%ld, permissions=%o", 
            file_size, owner_id, last_modified, permissions & 07777);
        build_response_t(resp, 200, "Successfuly found INFO for file.", data, data_size);
    } else {
        size_t data_size = sizeof(file_size) + 1;
        char* data = malloc(data_size);
        sprintf(data, "size=%ld", file_size);
        build_response_t(resp, 206, "Found partial INFO for file.", data, data_size);
    }
}

void handle_MD(char* dir_path, response_t* resp, char* original_request, char* drive1_path, char* drive2_path) {
    bool drive1_success = handle_MD_drive(dir_path, drive1_path);
    bool drive2_success = handle_MD_drive(dir_path, drive2_path);

    // write to response.
    if (drive1_success && drive2_success) {
        build_response_t(resp, 201, "Successfully created directory on both replicas.", NULL, 0);
    } else if (drive1_success) {
        create_journal_entry(original_request, 2);
        build_response_t(resp, 201, "Successfully created directory on replica 1.", NULL, 0);
    } else if (drive2_success) {
        create_journal_entry(original_request, 1);
        build_response_t(resp, 201, "Successfully created directory on replica 2.", NULL, 0);
    } else {
        build_response_t(resp, 500, "Unable to create directory.", NULL, 0);
    }
}

void handle_PUT(char* file_path, void* content, long content_size, response_t* resp, char* original_request, char* drive1_path, char* drive2_path) {
    // Check that drives have space to create the file.
    if (!check_drive_free_space(content_size, drive1_path)) {
        build_response_t(resp, 500, "Not enough space to create file on drive 1.", NULL, 0);
        return;
    } else if (!check_drive_free_space(content_size, drive2_path)) {
        build_response_t(resp, 500, "Not enough space to create file on drive 2.", NULL, 0);
        return;
    }

    bool drive1_success = handle_PUT_drive(file_path, content, content_size, drive1_path);
    bool drive2_success = handle_PUT_drive(file_path, content, content_size, drive2_path);

    // write to response.
    if (drive1_success && drive2_success) {
        build_response_t(resp, 201, "Successfully put file.", NULL, 0);
    } else if (drive1_success) {
        create_journal_entry(original_request, 2);
        build_response_t(resp, 201, "Successfully put file on replica 1.", NULL, 0);
    } else if (drive2_success) {
        create_journal_entry(original_request, 1);
        build_response_t(resp, 201, "Successfully put file on replica 2.", NULL, 0);
    } else {
        build_response_t(resp, 500, "Could not create file.", NULL, 0);
    }
}

void handle_RM(char* path, response_t* resp, char* original_request, char* drive1_path, char* drive2_path) {
    bool drive1_success = handle_RM_drive(path, drive1_path);
    bool drive2_success = handle_RM_drive(path, drive2_path);

    // write to response.
    if (drive1_success && drive2_success) {
        build_response_t(resp, 200, "Successfully removed item.", NULL, 0);
    } else if (drive1_success) {
        create_journal_entry(original_request, 2);
        build_response_t(resp, 201, "Successfully removed item on replica 1.", NULL, 0);
    } else if (drive2_success) {
        create_journal_entry(original_request, 1);
        build_response_t(resp, 201, "Successfully removed item on replica 2.", NULL, 0);
    } else {
        build_response_t(resp, 500, "Could not remove item.", NULL, 0);
    }
}

int bind_server_socket(char* server_ip_address, char* server_port_string) {
    // Try to read server settings from file.
    FILE *file = fopen(SERVER_CONNECTION_SETTINGS, "r");
    if (file == NULL) {
        printf("Could not find connection settings file: %s", SERVER_CONNECTION_SETTINGS);
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

    printf("Server port = %d\n", server_port);
    printf("Server IP address = %s\n", server_ip_address);

    // Create socket.
    int server_socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_desc < 0) {
        printf("Server socket creation failed.\n");
        return -1;
    }
    printf("Server socket created successfully.\n");

    // Set the server port and IP.
    struct sockaddr_in server_info;
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(server_port);
    server_info.sin_addr.s_addr = inet_addr(server_ip_address);

    // Bind the socket to the given port and IP.
    if (bind(server_socket_desc, (struct sockaddr*)&server_info, sizeof(server_info)) < 0) {
        printf("Server unable to bind to port read from config file.\n");
        return -1;
    }
    printf("Server successfully bound to port = %d with IP = %s\n", server_port, server_ip_address);

    return server_socket_desc;
}