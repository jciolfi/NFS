#include "server_handler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * Check if given filepaths exist, use for checking state of file on both drives.
 * 
 * @param filepath1 the path for drive1.
 * @param filepath2 the path for drive2.
*/
void exists_on_both_drives(char* filepath1, char* filepath2) {
    bool exists1 = access(filepath1, F_OK) == 0;
    bool exists2 = access(filepath2, F_OK) == 0;
    if (exists1 && exists2) {
        printf("Both paths have a file that exists: %s AND %s\n", filepath1, filepath2);
    } else if (exists1) {
        printf("File exists on path1 but not path2: %s, but NOT %s\n", filepath1, filepath2);
    } else if (exists2) {
        printf("File exists on path2 but not path1: %s, but NOT %s\n", filepath2, filepath1);
    } else {
        printf("No file found for either path: %s OR %s\n", filepath1, filepath2);
    }
}


int main() {
    // establish paths for drives from config.
    char drive1_path[MAX_DRIVE_PATH];
    char drive2_path[MAX_DRIVE_PATH];
    set_drive_paths(drive1_path, drive2_path);

    char* PUT_req = "PUT file1.txt";
    printf("-- %s --\n", PUT_req);
    response_t PUT_resp;
    parse_request(PUT_req, &PUT_resp, drive1_path, drive2_path);
    print_response_t(&PUT_resp);
    char filepath1[MAX_DRIVE_PATH + 16];
    char filepath2[MAX_DRIVE_PATH + 16];
    sprintf(filepath1, "%s/%s", drive1_path, "file1.txt");
    sprintf(filepath2, "%s/%s", drive2_path, "file1.txt");
    exists_on_both_drives(filepath1, filepath2);


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
    sprintf(filepath1, "%s/%s", drive1_path, "tempdir");
    sprintf(filepath2, "%s/%s", drive2_path, "tempdir");
    exists_on_both_drives(filepath1, filepath2);
    
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
    sprintf(filepath1, "%s/%s", drive1_path, "tempdir");
    sprintf(filepath2, "%s/%s", drive2_path, "tempdir");
    exists_on_both_drives(filepath1, filepath2);

    char* PUT_req1 = "PUT newfile.txt";
    printf("-- %s --\n", PUT_req1);
    response_t PUT_resp1;
    parse_request(PUT_req1, &PUT_resp1, drive1_path, drive2_path);
    print_response_t(&PUT_resp1);
    sprintf(filepath1, "%s/%s", drive1_path, "newfile.txt");
    sprintf(filepath2, "%s/%s", drive2_path, "newfile.txt");
    exists_on_both_drives(filepath1, filepath2);

    char* PUT_req2 = "PUT newfile.txt some text for newfile.txt";
    printf("-- %s --\n", PUT_req2);
    response_t PUT_resp2;
    parse_request(PUT_req2, &PUT_resp2, drive1_path, drive2_path);
    print_response_t(&PUT_resp2);
    sprintf(filepath1, "%s/%s", drive1_path, "newfile.txt");
    sprintf(filepath2, "%s/%s", drive2_path, "newfile.txt");
    exists_on_both_drives(filepath1, filepath2);

    return 0;
}