#include "util_worker.h"

char * verify_result(char ** result) {
    DPRINT(printf("STDERR: %s\n", result[1]));
    DPRINT(printf("RUNTIMEERROR: %d\n", atoi(result[2])));
    DPRINT(printf("STDOUTlen: %ld\n", strlen(result[0])));
    DPRINT(printf("STDERRlen: %ld\n", strlen(result[1])));
    if (atoi(result[2]) == 0 || strlen(result[1]) > 0)
        return NULL;
    else
        return result[0];
}

/* TODO
timer should calculate execution time to test target program execution time exceeds 3 secs.
IF build success, execute success => return output to socket
IF build success, execute fail => return failure message to socket
             RUN TIME ERROR
             TIME EXCEED ERROR
IF build fail => return failure message to socket
*/

void build_target() {
    char ** result;
    char * CMD_BUILD[] = {
        COMPILER, TARGET_FILE, 
        "-O2", "-lm", "-static", "-DONLINE_JUDGE", "-DBOJ", NULL
    };
    char * CMD_EXECUTE_TARGET[] = { OUTPUT_FILE, NULL };

    result = execute_get_result(CMD_BUILD);

    if (strlen(result[1]) == 0){
        result = execute_get_result(CMD_EXECUTE_TARGET);
        if (verify_result(result) == NULL) {
            /* RUNTIME ERROR */
        } else {
            /* PROGRAM EXIT NORMALLY
             * RETURN OUTPUT TO INSTAGRAPD
             */
        }
        remove(OUTPUT_FILE);
    } else {
        fputs("BUILD FAIL", stderr);
    }
    remove(TARGET_FILE);
    remove(TESTCASE_FILE);
}

int main(int argc, char *argv[])
{
    receive_csrc_testcase(argc, argv);
    build_target();

    return 0;
}
