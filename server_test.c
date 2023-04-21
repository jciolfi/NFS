#include "server_handler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    // establish paths for drives from config.
    char drive1_path[MAX_DRIVE_PATH];
    char drive2_path[MAX_DRIVE_PATH];
    set_drive_paths(drive1_path, drive2_path);

    char* GET_req1 = "GET file1bad.txt";
    printf("-- %s --\n", GET_req1);
    response_t GET_resp1;
    parse_request(GET_req1, &GET_resp1, drive1_path, drive2_path);
    print_response_t(&GET_resp1);

    char* GET_req2 = "GET file1.txt";
    printf("-- %s --\n", GET_req2);
    response_t GET_resp2;
    parse_request(GET_req2, &GET_resp2, drive1_path, drive2_path);
    print_response_t(&GET_resp2);

    char* INFO_req1 = "INFO file1bad.txt";
    printf("-- %s --\n", INFO_req1);
    response_t INFO_resp1;
    parse_request(INFO_req1, &INFO_resp1, drive1_path, drive2_path);
    print_response_t(&INFO_resp1);

    char* INFO_req2 = "INFO file1.txt";
    printf("-- %s --\n", INFO_req2);
    response_t INFO_resp2;
    parse_request(INFO_req2, &INFO_resp2, drive1_path, drive2_path);
    print_response_t(&INFO_resp2);

    char* MD_req1 = "MD file.txt/tempdir";
    printf("-- %s --\n", MD_req1);
    response_t MD_resp1;
    parse_request(MD_req1, &MD_resp1, drive1_path, drive2_path);
    print_response_t(&MD_resp1);

    char* MD_req2 = "MD tempdir";
    printf("-- %s --\n", MD_req2);
    response_t MD_resp2;
    parse_request(MD_req2, &MD_resp2, drive1_path, drive2_path);
    print_response_t(&MD_resp2);
    
    char* RM_req1 = "RM tempdirbad";
    printf("-- %s --\n", RM_req1);
    response_t RM_resp1;
    parse_request(RM_req1, &RM_resp1, drive1_path, drive2_path);
    print_response_t(&RM_resp1);

    char* RM_req2 = "RM tempdir";
    printf("-- %s --\n", RM_req2);
    response_t RM_resp2;
    parse_request(RM_req2, &RM_resp2, drive1_path, drive2_path);
    print_response_t(&RM_resp2);

    char* PUT_req1 = "PUT newfile.txt";
    printf("-- %s --\n", PUT_req1);
    response_t PUT_resp1;
    parse_request(PUT_req1, &PUT_resp1, drive1_path, drive2_path);
    print_response_t(&PUT_resp1);

    char* PUT_req2 = "PUT newfile.txt some text for newfile.txt";
    printf("-- %s --\n", PUT_req2);
    response_t PUT_resp2;
    parse_request(PUT_req2, &PUT_resp2, drive1_path, drive2_path);
    print_response_t(&PUT_resp2);

    return 0;
}