#include <string.h>

#include "util_worker.h"

int  verify_result(char ** result) {
    DPRINT(printf("STDERR: %s\n", result[1]));
    DPRINT(printf("RUNTIMEERROR: %d\n", atoi(result[2])));
    DPRINT(printf("STDOUTlen: %ld\n", strlen(result[0])));
    DPRINT(printf("STDERRlen: %ld\n", strlen(result[1])));

    if (atoi(result[2]) == 0 || strlen(result[1]) > 0)
        return -1;
    else
        return 0;
}

/* TODO
timer should calculate execution time to test target program execution time exceeds 3 secs.
IF build success, execute success => return output to socket
IF build success, execute fail => return failure message to socket
             RUN TIME ERROR
             TIME EXCEED ERROR
                refer: httpe://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rttims.htm
IF build fail => return failure message to socket
*/

char * build_target() {
    char ** result;
    char * feedback;
    char * CMD_BUILD[] = {
        COMPILER, TARGET_FILE, 
        "-O2", "-lm", "-static", "-DONLINE_JUDGE", "-DBOJ", NULL
    };
    char * CMD_EXECUTE_TARGET[] = { OUTPUT_FILE, NULL };

    result = execute_get_result(CMD_BUILD);
    feedback = (char *) malloc(sizeof(char));

    if (strlen(result[1]) == 0){
        result = execute_get_result(CMD_EXECUTE_TARGET);
        if (verify_result(result) == -1) {
            /* RUNTIME ERROR */
            strcpy(feedback, "2");
        } else {
            /* PROGRAM EXIT NORMALLY
             * RETURN OUTPUT TO INSTAGRAPD
             */
            free(feedback);
            feedback = (char *) malloc(sizeof(char) * strlen(result[0]) + 1);
            strcpy(feedback, "0");
            strcat(feedback, result[0]);
        }
        remove(OUTPUT_FILE);
    } else {
        strcpy(feedback, "1");
    }
    remove(TARGET_FILE);
    remove(TESTCASE_FILE);
    return feedback;
}

int main(int argc, char *argv[])
{
    receive_csrc_testcase(argc, argv);
    printf("%s\n", build_target());

    return 0;
}
