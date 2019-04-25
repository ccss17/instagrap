#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "instagrap.h"

const char * ARG_PARSER = ":p:h";
static const char TARGET_FILE[] = "target.c";

void need_help() { fprintf(stderr, "enter './worker -h' for help message\n"); }
void help() { fprintf(stderr, "Usage: ./worker -p <PORT> \n"); }

void * worker(void * pclnt_sd) {
    int clnt_sd;
    char ** result;
    char * feedback;
    char rand_str[8];
    char TARGET_FILE[30];
    data_set * testcase;
    data_set * targetc;

    clnt_sd = * (int *)pclnt_sd;
#if DEBUG
    printf("clnt_sd : %d\n", clnt_sd);
#endif

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
        data_set * return_output = (data_set *) malloc(sizeof(data_set));
        return_output->size = strlen(feedback + 1);
        return_output->data = feedback+1;
        send_dataset(clnt_sd, return_output);
    }

#if ! DEBUG
    /*free(rand_str);*/
    remove(TARGET_FILE);
#endif
    /*free(feedback);*/
    /*free(result);*/
    shutdown(clnt_sd, SHUT_WR); 
    close(clnt_sd);
    printf("#######################################\n");
}

int main( int argc, char *argv[] )
{
    if (argc == 1) {
        help();
        return 1;
    }

    int opt;
    int serv_sd;
    int clnt_sd;
    int threads_index = 0;
    /*pthread_t * threads;*/
    pthread_t threads[THREAD_LIMIT];
    /*int thread_queue[THREAD_LIMIT];*/

    /*memset(thread_queue, 0, sizeof(int) * THREAD_LIMIT);*/
    
    while((opt = getopt(argc, argv, ARG_PARSER)) != -1){
        switch(opt) {
            case 'p':
                serv_sd = init_serv_sock(optarg);
                break;
            case 'h':
                help();
                exit(1);
            case ':': 
                fprintf(stderr, "option needs a value\n");
                exit(1);
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); 
                break; 
        }  
    }

    if (serv_sd == -1) {
        fprintf(stderr, "error occur when initializing server socket\n");
        return 1;
    }

    while (1) {
        clnt_sd = accept_connection(serv_sd);
        pthread_create(&threads[threads_index], NULL, worker, (void*) &clnt_sd);
        threads_index++;
        if (threads_index == THREAD_LIMIT) 
            threads_index = 0;
    }
    
    return 0;
}
