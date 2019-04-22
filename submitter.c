#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "instagrap.h"

const char * ARG_PARSER = ":n:u:k:h";

typedef struct {
    char * instagrapd_ip; 
    char * instagrapd_port; 
    char * stdid; 
    char * pw; 
    char * path_targetc;
} submitter_args;

void need_help() { fprintf(stderr, "enter './submitter -h' for help message\n"); }
void help_usage() { fprintf(stderr, "Usage: ./submitter -n <IP>:<PORT> -u <ID> -k <PW> <FILE>\n"); }

void * submitter(void * arg) {
    submitter_args * args = (submitter_args *) arg;
    int instagrapd_sock;
    data_set * targetc;
    char * flag;

#if DEBUG
    printf("%s\n", args->instagrapd_ip);
    printf("%s\n", args->instagrapd_port);
    printf("%s\n", args->stdid);
    printf("%s\n", args->pw);
    printf("%s\n", args->path_targetc);
#endif

    instagrapd_sock = establish_connection(args->instagrapd_ip, args->instagrapd_port);
    write(instagrapd_sock, args->stdid, strlen(args->stdid));
    write(instagrapd_sock, args->pw, strlen(args->pw));

    targetc = readfile(args->path_targetc);
    send_dataset(instagrapd_sock, targetc);

    flag = (char *)malloc(sizeof(char) * 3);
    read(instagrapd_sock, flag, 1);
    flag[1] = '\0';
    switch(flag[0]) {
        case '0':
            read(instagrapd_sock, flag, BUF_SIZE);
            flag[2] = '\0';
            printf("result:%s/%ld\n", flag, strlen(flag));
            break;
        case '1':
            printf("BUILD FAILED");
            break;
        case '2':
            printf("RUNTIME ERROR");
            break;
        case '3':
            printf("TIMEOUT ERROR");
            break;
        case '4':
            printf("INCORRECT PASSWORD ERROR");
            break;
        default:
            printf("UNKNOWN ERROR:%s\n", flag);
            break;
    }
}

int main(int argc, char * argv[]) {
    if (argc == 1) {
        help_usage();
        return 1;
    }

    int opt;
    char ** ipport;
    submitter_args * args;

    args = (submitter_args *) malloc(sizeof(submitter_args));

    while((opt = getopt(argc, argv, ARG_PARSER)) != -1) {
        switch(opt) {
            case 'n':
                ipport = str_split(optarg, ':');
                args->instagrapd_ip = *(ipport);
                args->instagrapd_port = *(ipport+1);
                break;  
            case 'u':
                args->stdid = optarg;
                break;  
            case 'k':
                args->pw = optarg;
                break;  
            case 'h':
                help_usage();
                exit(1);
            case ':':  
                help_usage();
                exit(1);
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); break;  
        }
    }

    if (argc - optind != 1 ) {
        need_help();
        return 1;
    }

    args->path_targetc = argv[optind];

    submitter((void *) args);

    return 0;
}
