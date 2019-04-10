#ifndef _UTIL_WORKER
#define _UTIL_WORKER

#include <stdlib.h>
#include <stdio.h>

#define DEBUG 0
#if DEBUG
    #define DPRINT(func) func
#else
    #define DPRINT(func)
#endif

#define FILE_SIZE_INDICATOR 4
#define BUF_SIZE 512
#define OUTPUT_BUF BUF_SIZE * 10
#define PIPE_COUNT 4

#define TARGET_FILE "target.c"
#define TESTCASE_FILE "test.in"
#define OUTPUT_FILE "./a.out"
#define COMPILER "/usr/bin/gcc"

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
char * execute_get_output(const char * cmd) ;
int closecmd(const pid_t pid, int *pipes);
char ** execute_get_result(char * args[]);
pid_t _execute(int * pipes, char ** args);
char * read_from_pipe(int pfd, int flag) ;

#endif
