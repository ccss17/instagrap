#include "util_worker.h"

#define DEBUG 0
#if DEBUG
#define DPRINT(func) func
#else
#define DPRINT(func)
#endif


//
// TEST FUNCTION
//

void TEST_build_target_file() {
    if ( ! file_exists(TARGET_FILE)) {
        error_handling("TARGET FILE doesn't exists");
    }
    FILE * fp;
    char path[BUF_SIZE];
    char cmd[BUF_SIZE];

    strcpy(cmd, "gcc");
    strcat(cmd, " ");
    strcat(cmd, TARGET_FILE);
    strcat(cmd, " ");
    strcat(cmd, "2>&1");

    fp = popen(cmd, "r");
    if (fp == NULL) {
        error_handling ("Failed to run command");
        exit(1);
    }
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        /*printf("%s", path);*/
    }
    pclose(fp);

    if (file_exists(OUTPUT_FILE)) {
        printf("build success\n");
        remove(OUTPUT_FILE);
    }
    else
        printf("build fail\n");
}

//
// MAIN
//

int main(int argc, char *argv[])
{
    receive_csrc_testcase(argc, argv);
    /*TEST_build_target_file() ;*/

    return 0;
}
