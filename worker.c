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
    write(sc_sd->clnt_sd, result, 1);
    if ( *result == '0' ) {
        result++;
        write(sc_sd->clnt_sd, result, strlen(result));
    }

    cleanup_socket(sc_sd);
    return 0;
}
