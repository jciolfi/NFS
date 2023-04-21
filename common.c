/*
*  common.c / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#include <stdlib.h>
#include <string.h>
#include "common.h"


// ---------- Methods NOT defined in common.h ----------

/**
 * Return a serialized string for a response_t object, assumed to be non-null and well formed.
 * 
 * @param response the response to serialize.
 * @param buffer the buffer to place the serialized response in.
 * @param buffer_size the length of the buffer.
 * @return the serialized response.
*/
char* serialize_response_t_help(response_t* response, char* buffer, unsigned long buffer_size) {
    long offset = 0;

    // set message length in buffer.
    unsigned long* size_ptr = (unsigned long*) buffer;
    *size_ptr = buffer_size;
    offset += sizeof(unsigned long);

    // set status in buffer.
    int* status_ptr = (int*) (buffer + offset);
    *status_ptr = response->status;
    offset += sizeof(int);

    // set message portion of buffer.
    char* message_ptr = buffer + offset;
    memcpy(message_ptr, response->message, MESSAGE_SIZE);
    offset += MESSAGE_SIZE;

    // set data size portion of buffer.
    size_t* data_size_ptr = (size_t*) (buffer + offset);
    *data_size_ptr = response->data_size;
    offset += sizeof(size_t);

    // set data portion of buffer.
    if (response->data != NULL && response->data_size > 0) {
        void* data_ptr = buffer + offset;
        memcpy(data_ptr, response->data, response->data_size);
    }

    return buffer;
}

// ---------- Methods defined in common.h ----------

void build_response_t(response_t* resp, int status, char* message, void* data, size_t data_size) {
    // assign status and data for the response.
    resp->status = status;
    resp->data = data;
    resp->data_size = data_size;

    // assign the message for the response
    int message_len = strlen(message) < sizeof(resp->message) - 1
        ? strlen(message)
        : sizeof(resp->message) - 1;
    strncpy(resp->message, message, message_len);
    resp->message[message_len] = '\0';
}

char* serialize_response_t(response_t* response) {
    // allocate buffer
    unsigned long buffer_size = sizeof(unsigned long) + sizeof(response_t) + (response == NULL ? 0 : response->data_size);
    char* buffer = (char*) malloc(buffer_size);

    // handle null response
    if (response == NULL) {
        response_t temp_response;
        build_response_t(&temp_response, 500, "Unable to serialize null response.", NULL, 0);
        char* result = serialize_response_t_help(&temp_response, buffer, buffer_size);
        return result;
    }
    
    return serialize_response_t_help(response, buffer, buffer_size);
}

void deserialize_response_t(char* server_resp, response_t* response) {
    long offset = sizeof(unsigned long);
    
    // extract status.
    int* status_ptr = (int*) (server_resp + offset);
    response->status = *status_ptr;
    offset += sizeof(int);

    // extract message.
    char* message_ptr = server_resp + offset;
    memcpy(response->message, message_ptr, MESSAGE_SIZE);
    offset += MESSAGE_SIZE;

    // extract data size.
    size_t* data_size_ptr = (size_t*) (server_resp + offset);
    response->data_size = *data_size_ptr;
    offset += sizeof(size_t);

    // extract data if applicable.
    if (response->data_size > 0) {
        void* data_ptr = server_resp + offset;
        response->data = data_ptr;
    } else {
        response->data = NULL;
    }
}

action_t get_action(char* request) {
    char action[8];
    unsigned long i;
    for (i = 0; i < strlen(request); i++) {
        // the longest action is 4 characters long.
        if (i >= 5) {
            return NOT_FOUND;
        }

        // parse request until a space is seen.
        if (request[i] != ' ') {
            action[i] = request[i];
        } else {
            action[i] = '\0';
            break;
        }
    }

    if (strcmp(action, "GET") == 0) {
        return GET;
    } else if (strcmp(action, "INFO") == 0) {
        return INFO;
    } else if (strcmp(action, "MD") == 0) {
        return MD;
    } else if (strcmp(action, "PUT") == 0) {
        return PUT;
    } else if (strcmp(action, "RM") == 0) {
        return RM;
    } else {
        return NOT_FOUND;
    }
}

void substring(char* str, unsigned long start_idx, unsigned long end_idx, char* substr) {
    if (str == NULL || substr == NULL || start_idx >= strlen(str) || end_idx == 0) {
        return;
    }

    // copy substring into buffer.
    int length = end_idx - start_idx;
    strncpy(substr, str + start_idx, length);
    substr[length] = '\0';
}

void print_response_t(response_t* response) {
    printf("Server Response:\n  Status: %d\n  Message: %s\n  Data: %s\n",
        response->status, response->message, (char*) response->data);
}