#include <stdio.h>
#include "csapp.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct {
    char http_req_line[128];
    char http_req_header[1024];
} http_req;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* This function receive the connection with a client */
void recv_client_req(char *listen_port);

/* Parse the client request content, if success return 0, else return -1 */
int parse_cli_req(char *req_str, http_req *req_struct);

/* Make connection to the actual server*/
int main(int argc, char *argv[])
{
    // If the user enter less than 2 arguments, reminds the user to enter
    if (argc != 2) {
        fprintf(stderr, "usage %s <port number>", argv[0]);
        exit(0);
    }

    recv_client_req(argv[1]);

    printf("%s", user_agent_hdr);
    return 0;
}

void recv_client_req(char *listen_port)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    
    listenfd = Open_listenfd(listen_port);
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, &clientaddr, &clientlen);
    printf("Client connected!!\n");

    size_t n;
    char buf[1024];
    rio_t rio;
    http_req req_content;

    Rio_readinitb(&rio, connfd);
    n = Rio_readlineb(&rio, buf, 1024);
    printf("%s", buf);

    parse_cli_req(buf, &req_content);

    char *ret = "Hello World\r\n\r\n";

    Rio_writen(connfd, ret, strlen(ret));

    Close(connfd);
}

int parse_cli_req(char *req_str, http_req *req_struct)
{
    
}
