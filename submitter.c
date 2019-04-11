#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMD_SIZE 128

const char * ARG_PARSER = ":n:u:k:h";

void need_help() {
    fprintf(stderr, "enter './submitter -h' for help message\n");
}

void help_usage() {
    fprintf(stderr, "Usage: ./submitter -n <IP>:<PORT> -u <ID> -k <PW> <FILE>\n");
}

void argparse(int argc, char * argv[]) {
    int opt;
    while((opt = getopt(argc, argv, ARG_PARSER)) != -1)  
    {
        switch(opt)  
        {
            case 'n':
                printf("option n:%s\n", optarg);
                /*process_ipport(optarg);*/
                break;  
            case 'u':
                printf("option u:%s\n", optarg);
                /*process_stdid(optarg);*/
                break;  
            case 'k':
                printf("option k:%s\n", optarg);
                /*process_password(optarg);*/
                break;  
            case 'h':
                help_usage();
                break;  
            case ':':  
                help_usage();
                break;  
            case '?':  
                fprintf(stderr, "unknown option: %c\n", optopt); 
                break;  
        }  
    }
}

int main(int argc, char * argv[]) {
    if (argc == 1) {
        help_usage();
        return 1;
    }

    argparse(argc, argv);

    if (argc - optind != 1 ) {
        need_help();
        return 1;
    }

    // process_csource(argv[optind]);
    printf("filename:%s\n", argv[optind]);

    return 0;
}
