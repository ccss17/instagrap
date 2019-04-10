#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "instagrap.h"

/* TODO
 * deliver test.in to a.out
 * receive requests forever
 */

int main(int argc, char *argv[])
{
    sock_set * sc_sd;
    char * result;

    sc_sd = init_sock(argc, argv);
    get_file(TARGET_FILE, sc_sd->clnt_sd);
    get_file(TESTCASE_FILE, sc_sd->clnt_sd);

    result = build_target();
    switch (*result) {
        case '0':
            // PROGRAM EXIT NORMALLY
            write(sc_sd->clnt_sd, "0", 1);
            result++;
            write(sc_sd->clnt_sd, result, strlen(result));
            break;
        case '1':
            // BUILD FAILED
            write(sc_sd->clnt_sd, "1", 1);
            break;
        case '2':
            // RUNTIME ERROR
            write(sc_sd->clnt_sd, "2", 1);
            break;
        case '3':
            // TIMEOUT ERROR
            write(sc_sd->clnt_sd, "3", 1);
            break;
    }

    cleanup_socket(sc_sd);
    return 0;
}
