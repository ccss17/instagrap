#ifndef _UTIL_WORKER
#define _UTIL_WORKER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEBUG 0
#if DEBUG
    #define DPRINT(func) func
#else
    #define DPRINT(func)
#endif

#define FILE_SIZE_INDICATOR 4
#define BUF_SIZE 512
#define TARGET_FILE "target.c"
#define TESTCASE_FILE "test.in"
#define OUTPUT_FILE "a.out"

typedef struct {
    int serv_sd;
    int clnt_sd;
} sock_set;

void error_handling(char *message);
int get_file_size(int clnt_sd) ;
void get_save_file(const char * filename, int clnt_sd );
void _save_file(FILE * fp, int clnt_sd, char buf[], int size, int read_cnt) ;
sock_set * init_accept_socket(int argc, char * argv[]) ;
void cleanup_socket(sock_set * sc_sd) ;
int file_exists(const char * filename) ;
void receive_csrc_testcase(int argc, char * argv[]) ;

#endif
