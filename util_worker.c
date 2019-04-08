#include "util_worker.h"
//
// IMPLEMENT UTIL FUNCTION
//

void receive_csrc_testcase(int argc, char * argv[]) {
    // INIT
    sock_set * sc_sd;
    sc_sd=init_accept_socket(argc, argv);

    // FUNCTION
    get_save_file(TARGET_FILE, sc_sd->clnt_sd);
    get_save_file(TESTCASE_FILE, sc_sd->clnt_sd);

    // CLEAN UP
    cleanup_socket(sc_sd);
}

int file_exists(const char * filename) {
    FILE * file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

void cleanup_socket(sock_set * sc_sd) {
    shutdown(sc_sd->clnt_sd, SHUT_WR); 
    close(sc_sd->clnt_sd);
    close(sc_sd->serv_sd);
}

sock_set * init_accept_socket(int argc, char * argv[]) {
    sock_set * sc_sd;
    int serv_sd, clnt_sd;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sc_sd = (sock_set *) malloc(sizeof(sock_set));

    sc_sd->serv_sd = socket(PF_INET, SOCK_STREAM, 0);   
    if (sc_sd->serv_sd == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(sc_sd->serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if(listen(sc_sd->serv_sd, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz=sizeof(clnt_adr);    
    sc_sd->clnt_sd=accept(sc_sd->serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    return sc_sd;
}

int get_file_size(int clnt_sd) {
    char buf[FILE_SIZE_INDICATOR];
    read(clnt_sd, buf, FILE_SIZE_INDICATOR );
    return atoi(buf);
}

void get_save_file(const char * filename, int clnt_sd ){
    FILE * fp;
    char buf[BUF_SIZE];
    int read_cnt;
    int file_size;

    fp = fopen(filename, "wb");
    file_size = get_file_size(clnt_sd);
    DPRINT(printf("file size : %d\n", file_size));

    while(1){
        if (file_size < BUF_SIZE) {
            _save_file(fp, clnt_sd, buf, file_size, read_cnt);
            break;
        } else {
            file_size -= BUF_SIZE;
            _save_file(fp, clnt_sd, buf, BUF_SIZE, read_cnt);
        }
    }

    fclose(fp);
}

void _save_file(FILE * fp, int clnt_sd, char buf[], int size, int read_cnt) {
    read_cnt=read(clnt_sd, buf, size );
    DPRINT(printf("buf : %s\n", buf));
    fwrite((void*)buf, 1, read_cnt, fp);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
