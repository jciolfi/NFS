#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "client_handler.h"

#define NUM_THREADS 8

void* put_file1(void* data) {
    int thread_idx = *(int*)data;

    // Connect to the server using TCP sockets.
    char* server_ip_address = malloc(sizeof(char) * 30);
    char* server_port_string = malloc(sizeof(char) * 10);
    int socket_desc = connect_to_server(server_ip_address, server_port_string);
    if (socket_desc < 0) {
        // Connection failed.
        return NULL;
    }
    printf("Thread %d successfully connected.\n", thread_idx);

    // handle request.
    if (thread_idx % 2 == 0) {
        // update file for this thread.
        char file_name[16];
        sprintf(file_name, "temp%d.txt", thread_idx);
        FILE* file = fopen(file_name, "w");
        if (file == NULL) {
            printf("Couldn't open %s\n", file_name);
            return NULL;
        }
        fprintf(file, "UPDATED BY THREAD ** %d **", thread_idx);
        fclose(file);

        char* args1[] = {"./fget", "PUT", file_name, "file1.txt"};
        parse_fget(4, args1, socket_desc);
    } else {
        usleep(600);
        char* args2[] = {"./fget", "GET", "file1.txt"};
        parse_fget(3, args2, socket_desc);
    }

    free(server_ip_address);
    free(server_port_string);
    close(socket_desc);
    return NULL;
}

int main(void) {
    // initialize threads.
    pthread_t threads[NUM_THREADS];
    int thread_idxs[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_idxs[i] = i;
        pthread_create(&threads[i], NULL, put_file1, &thread_idxs[i]);
    }

    // join threads threads so they finish.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}