#ifndef _INSTAGRAP
#define _INSTAGRAP

#include <stdlib.h>
#include <stdio.h>

#define DEBUG 1

#define TESTCASE_COUNT 10
#define IDENTIFIER_SIZE 8
#define THREAD_LIMIT 30
#define FILE_SIZE_INDICATOR 4
#define BUF_SIZE 128
#define OUTPUT_BUF BUF_SIZE * 10
#define PIPE_COUNT 4
#define TIMEOUT 3

#define DEFAULT_OUTPUT_FILE "./a.out"
#define COMPILER "/usr/bin/gcc"

typedef struct {
    size_t size;
    char * data;
} data_set;

typedef struct {
    int serv_sd;
    int clnt_sd;
} sock_set;


char * itoa(int integer) ;
void send_dataset(int sock, data_set * ds) ;
int establish_connection(char * ip, char * port) ;
data_set * readfile(char *filename) ;
int accept_connection(int serv_sd) ;
int init_serv_sock(char * listen_port);
data_set * receive_data( int sock ) ;
//void save_file(char * filename, data_set *ydata_s) ;
int save_file(char * filename, data_set *data_s) ;

int build(char * build_target) ;
char ** execute(char * args[], char * input);
pid_t _fork_subprocess(int * pipes, char ** args, char * input);
char * _read_pipe(int pfd, int flag) ;
int  verify_result(char ** result) ;

void error_handling(char *message) ;
int closecmd(const pid_t pid, int *pipes);
char** str_split(char* a_str, const char a_delim) ;
int dir_exists(char * dirname) ;
int exists(char * fname) ;
void rand_string(char *str, size_t size) ;
#endif
