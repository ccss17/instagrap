#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEBUG 1
#ifdef DEBUG
#define DPRINT(func) func
#else
#define DPRINT(func)
#endif

#define FILE_SIZE_INDICATOR 4
#define BUF_SIZE 8

void error_handling(char *message);
int get_file_size(int clnt_sd) ;
void write_file(char * filename, int clnt_sd );

int main(int argc, char *argv[])
{
    // INIT
    int serv_sd, clnt_sd;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sd=socket(PF_INET, SOCK_STREAM, 0);   
    if (serv_sd == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if(listen(serv_sd, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz=sizeof(clnt_adr);    
    clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    // FUNCTION
    write_file("target.c", clnt_sd);

    // CLEAN UP
    shutdown(clnt_sd, SHUT_WR); 

    close(clnt_sd);
    close(serv_sd);
    return 0;
}

int get_file_size(int clnt_sd) {
    char buf[FILE_SIZE_INDICATOR];
    read(clnt_sd, buf, FILE_SIZE_INDICATOR );
    return atoi(buf);
}

void write_file(char * filename, int clnt_sd ){
    FILE * fp;
    char buf[BUF_SIZE];
    int read_cnt;
    int file_size;

    file_size = get_file_size(clnt_sd);
    fp = fopen(filename, "wb");
    DPRINT(printf("file size : %d\n", file_size));

    while(1){
        if (file_size < BUF_SIZE) {
            read_cnt=read(clnt_sd, buf, file_size );
            DPRINT(printf("buf : %s\n", buf));
            fwrite((void*)buf, 1, read_cnt, fp);
            DPRINT(puts("break"));
            break;
        } else {
            file_size -= BUF_SIZE;
            read_cnt=read(clnt_sd, buf, BUF_SIZE );
            DPRINT(printf("buf : %s\n", buf));
            fwrite((void*)buf, 1, read_cnt, fp);
        }
    }

    fclose(fp);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
