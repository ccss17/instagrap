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

    sc_sd = init_sock(argc, argv);

    testcase = receive_data(sc_sd->clnt_sd);
    DPRINT(printf("testcase.in:%s\n", testcase->data));
    save_file(TARGET_FILE, receive_data(sc_sd->clnt_sd));

    if( build(TARGET_FILE) == 0 ) {
        // BUILD SUCCESS
        feedback = (char *)malloc(sizeof(char));
        result = execute((char *[]){ DEFAULT_OUTPUT_FILE, NULL });
        switch(verify_result(result)) {
            case -1:
                DPRINT(printf("%s\n", result[1]));
                // RUNTIME ERROR 
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
    } else {
        // BUILD FAIL
        strcpy(feedback, "1");
    }
    write(sc_sd->clnt_sd, feedback, 1);
    if ( *feedback == '0' ) {
        feedback++;
        write(sc_sd->clnt_sd, feedback, strlen(feedback));
    }
    free(feedback);
    free(result);

    cleanup_socket(sc_sd);
    return 0;
}
