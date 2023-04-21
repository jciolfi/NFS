/*
*  common.h / Assignment: Practicum 2
*
*  James Florez and John Ciolfi / CS5600 / Northeastern University
*  Spring 2023 / Apr 9, 2023
*/

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define MESSAGE_SIZE 128
#define SERVER_ACTIONS "GET, INFO, MD, PUT, RM"

typedef struct fget_response {
    int status;
    char message[MESSAGE_SIZE];
    void* data;
    size_t data_size;
} response_t;

typedef enum fget_action {
    NOT_FOUND,
    GET,
    INFO,
    MD,
    PUT,
    RM
} action_t;


/**
 * Build response_t based on the given fields. Will allocate memory.
 * 
 * @param resp the response_t to write to.
 * @param status the status code for the response.
 * @param message the message with info for the response.
 * @param data the data with the request, such as a file.
 * @param data_size the size in bytes of the data.
 * 
*/
void build_response_t(response_t* resp, int status, char* message, void* data, size_t data_size);

/**
 * Return a serialized string for a response_t object.
 * 
 * @param response the response to serialize.
 * @return the serialized response.
*/
char* serialize_response_t(response_t* response);

/**
 * Deserialize a response from the server.
 * 
 * @param server_resp the raw response from the server.
*/
void deserialize_response_t(char* server_resp, response_t* response);

/**
 * Get the action for a request.
 * 
 * @param request the request to parse.
 * @return the action sepcified in the request.
*/
action_t get_action(char* request);

/**
 * Write substring of str to substr from start_idx (inclusive) to end_idx (exclusive) of str.
 * Assumes buffer of substr is long enough.
 * 
 * @param str the string to slice.
 * @param start_idx the start index of the substirng.
 * @param 
 * @param substr the buffer to write to. Assumed to be large enough.
*/
void substring(char* str, unsigned long start_idx, unsigned long end_idx, char* substr);

/**
 * Print the status and message of the response.
 * 
 * @param response the response to print.
*/
void print_response_t(response_t* response);

#endif