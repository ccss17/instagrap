#include "util_worker.h"

void TEST_build_target_file() {
    if ( ! file_exists(TARGET_FILE)) {
        error_handling("TARGET FILE doesn't exists");
    }
    if ( ! file_exists(TESTCASE_FILE)) {
        error_handling("TESTCASE FILE doesn't exists");
    }
    FILE * fp;
    char output[BUF_SIZE];
    char cmd[BUF_SIZE];

    strcpy(cmd, "gcc");
    strcat(cmd, " ");
    strcat(cmd, TARGET_FILE);
    strcat(cmd, " ");
    strcat(cmd, "2>&1");

    char * output_1 = execute_get_output(cmd);
    printf("%s\n", output_1);

    if (file_exists(OUTPUT_FILE)) {
        printf("build success\n");
        strcpy(cmd, "./a.out");

        char * output_2 = execute_get_output(cmd);
        printf("%s\n", output_2);

        remove(OUTPUT_FILE);
        free(output_2);
    }
    else
        printf("build fail\n");

    remove(TARGET_FILE);
    remove(TESTCASE_FILE);
}

void TEST_check_status_message(char ** result) {
    DPRINT(printf("STDOUT: %s\n", result[0]));
    DPRINT(printf("STDERR: %s\n", result[1]));
    printf("RUNTIMEERROR: %d\n", atoi(result[2]));
    printf("STDOUTlen: %ld\n", strlen(result[0]));
    printf("STDERRlen: %ld\n", strlen(result[1]));
}

/* TODO
timer should calculate execution time to test target program execution time exceeds 3 secs.
IF build success, execute success => return output to socket
IF build success, execute fail => return failure message to socket
             RUN TIME ERROR
             TIME EXCEED ERROR
IF build fail => return failure message to socket
*/

int main(int argc, char *argv[])
{
    char * CMD_BUILD[] = {
        "/usr/bin/gcc", TARGET_FILE, 
        "-O2", "-lm", "-static", "-DONLINE_JUDGE", "-DBOJ", NULL
    };
    char * CMD_EXECUTE_TARGET[] = { "./a.out", NULL };
    char ** result;

    receive_csrc_testcase(argc, argv);
    result = execute_get_result(CMD_BUILD);

    if (strlen(result[1]) == 0){
        result = execute_get_result(CMD_EXECUTE_TARGET);
        TEST_check_status_message(result);

        remove(OUTPUT_FILE);
        remove(TARGET_FILE);
        remove(TESTCASE_FILE);
    } else {
        fputs("BUILD FAIL", stderr);
    }

    return 0;
}
