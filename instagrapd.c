#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char * ARG_PARSER = ":p:w:h";

void need_help() { fprintf(stderr, "enter './instagrapd -h' for help message\n"); }
void help_usage() { fprintf(stderr, "Usage: ./instagrapd -p <PORT> -w <IP>:<WPORT> <DIR>\n"); }

void argparse(int argc, char * argv[]) {
    int opt;
    while((opt = getopt(argc, argv, ARG_PARSER)) != -1)  
    {
        switch(opt)  
        {
            case 'p':
                printf("option p:%s\n", optarg);
                /*process_ipport(optarg);*/
                break;  
            case 'w':
                printf("option w:%s\n", optarg);
                /*process_stdid(optarg);*/
                break;  
            case 'h':
                help_usage();
                exit(1);
            case ':': 
                fprintf(stderr, "option needs a value\n");
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
        help_usage();
        return 1;
    }

    // process_csource(argv[optind]);
    printf("dir:%s\n", argv[optind]);

    return 0;
}
