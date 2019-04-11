#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "instagrap.h"

/* TODO
 * deliver test.in to a.out
 * receive requests forever
 */

int main( int argc, char *argv[] )
{
    sock_set * sc_sd;
    char ** result;
    char * feedback;
    data_set * testcase;

    if(argc!=2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sc_sd = init_sock(argv[1]);
    feedback = (char *)malloc(sizeof(char));
    testcase = receive_data(sc_sd->clnt_sd);
    DPRINT(printf("testcase.in:%s\n", testcase->data));
    save_file(TARGET_FILE, receive_data(sc_sd->clnt_sd));

    if( build(TARGET_FILE) == 0 ) {
        // BUILD SUCCESS
        result = execute((char *[]){ DEFAULT_OUTPUT_FILE, NULL });
        switch(verify_result(result)) {
            case -1:
                // RUNTIME ERROR 
                DPRINT(printf("case-RUNTIME ERROR\n"));
                DPRINT(printf("STDERR:%s\n", result[1]));
                strcpy(feedback, "2");
                break;
            case -2:
                // TIMEOUT ERROR 
                strcpy(feedback, "3");
                break;
            case 0:
                // PROGRAM EXIT NORMALLY
                // RETURN OUTPUT TO INSTAGRAPD
                free(feedback);
                feedback = (char *) malloc(sizeof(char) * strlen(result[0]) + 1);
                strcpy(feedback, "0");
                strcat(feedback, result[0]);
                break;
        }
        /*remove(DEFAULT_OUTPUT_FILE);*/
    } else {
        // BUILD FAIL
        strcpy(feedback, "1");
    }
    write(sc_sd->clnt_sd, feedback, 1);
    if ( *feedback == '0' ) {
        write(sc_sd->clnt_sd, feedback+1, strlen(feedback+1));
    }

    /*remove(TARGET_FILE);*/
    free(feedback);
    free(result);

    cleanup_socket(sc_sd);
    return 0;
}
