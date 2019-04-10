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
#define TIMEOUT 3

#define TARGET_FILE "target.c"
#define TESTCASE_FILE "test.in"
#define OUTPUT_FILE "./a.out"
#define COMPILER "/usr/bin/gcc"

typedef struct {
    int serv_sd;
    int clnt_sd;
} sock_set;

void error_handling(char *message) ;

sock_set * init_sock(int argc, char * argv[]) ;
void cleanup_socket(sock_set * sc_sd) ;
int _get_size(int clnt_sd) ;
void get_file(const char * filename, int clnt_sd );
void _get_file(FILE * fp, int clnt_sd, char buf[], int size, int read_cnt) ;

char * build_target() ;
pid_t _fork_subprocess(int * pipes, char ** args);
int closecmd(const pid_t pid, int *pipes);
char ** _execute(char * args[]);
char * _read_pipe(int pfd, int flag) ;
int  verify_result(char ** result) ;

#endif
