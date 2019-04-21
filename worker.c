#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "instagrap.h"

const char * ARG_PARSER = ":p:h";

void need_help() { fprintf(stderr, "enter './worker -h' for help message\n"); }
void help_usage() { fprintf(stderr, "Usage: ./worker -p <PORT> \n"); }

int argparse(int argc, char * argv[]) {
    int opt;
    while((opt = getopt(argc, argv, ARG_PARSER)) != -1)  
    {
        switch(opt)  
        {
            case 'p':
                return init_serv_sock(optarg);
            case 'h':
                help_usage();
                exit(1);
            case ':': 
                fprintf(stderr, "option needs a value\n");
                exit(1);
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); 
                break;  
        }  
    }
}

void worker(int serv_sd) {
    int clnt_sd;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;
    char ** result;
    char * feedback;
    data_set * testcase;
    data_set * targetc;

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    feedback = (char *)malloc(sizeof(char));
    testcase = receive_data(clnt_sd);
    targetc = receive_data(clnt_sd);
    save_file(TARGET_FILE, targetc);
#if DEBUG
    printf("testcase.in:%s\n", testcase->data);
    printf("testcase.in length:%ld\n", strlen(testcase->data));
    printf("target.c:%s\n", targetc->data);
    printf("target.c length:%ld\n", strlen(targetc->data));
#endif

    if( build(TARGET_FILE) == 0 ) {
        // BUILD SUCCESS
        result = execute((char *[]){ DEFAULT_OUTPUT_FILE, NULL }, testcase->data);
        switch(verify_result(result)) {
            case -1:
                // RUNTIME ERROR 
#if DEBUG
                printf("case-RUNTIME ERROR\n");
                printf("STDERR:%s\n", result[1]);
#endif
                strcpy(feedback, "2");
                break;
            case -2:
                // TIMEOUT ERROR 
                strcpy(feedback, "3");
                break;
            case 0:
                // PROGRAM EXIT NORMALLY
                // RETURN OUTPUT TO INSTAGRAPD
                /*free(feedback);*/
                feedback = (char *) malloc(sizeof(char) * strlen(result[0]) + 1);
                strcpy(feedback, "0");
                strcat(feedback, result[0]);
                break;
        }
#if ! DEBUG
        remove(DEFAULT_OUTPUT_FILE);
#endif
    } else {
        // BUILD FAIL
        strcpy(feedback, "1");
    }
    write(clnt_sd, feedback, 1);
    if ( *feedback == '0' ) {
        write(clnt_sd, feedback + 1, strlen(feedback + 1));
    }

#if ! DEBUG
    remove(TARGET_FILE);
#endif
    /*free(feedback);*/
    /*free(result);*/
    shutdown(clnt_sd, SHUT_WR); 
    close(clnt_sd);
}

int main( int argc, char *argv[] )
{
    if (argc == 1) {
        help_usage();
        return 1;
    }

    int serv_sd;
    
    serv_sd = argparse(argc, argv);
    if (serv_sd == -1) {
        fprintf(stderr, "error occur when initializing server socket\n");
        return -1;
    }
    while(1) worker(serv_sd);
    
    return 0;
}
