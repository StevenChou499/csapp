#include <stdio.h>
#include "csapp.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXLINE 1024

typedef struct {
    char http_method[16];
    char http_url[256];
    char http_version[16];
    char http_req_header[MAXLINE];
} http_req;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* This function receive the connection with a client */
void recv_client_req(char *listen_port);
/* Parse the client request content, if success return 0, else return -1 */
int parse_cli_req(char *req_str, http_req *req_struct);
/* Check the url, modify the http version and return the server port number*/
int check_url_port(http_req *req_struct);

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
    char *str_ptr = strstr(req_str, "\r\n");
    if (str_ptr != NULL) {
        str_ptr += 2;
        strncpy(req_struct->http_req_header, 
                str_ptr, 
                sizeof(req_struct->http_req_header) - 1);
        req_struct->http_req_header[sizeof(req_struct->http_req_header) - 2] = '\0';

        if (sscanf(req_str, "%s %s %s", 
                   req_struct->http_method, 
                   req_struct->http_url, 
                   req_struct->http_version) != 3) {
            fprintf(stderr, "Failed to parse HTTP request line.\n");
            return -1;
        }

        if (strcmp(req_struct->http_method, "GET") != 0)
            return -1;
    } else {
        fprintf(stderr, "Failed to find the end of request line.\n");
        return -1;
    }
    return 0;
}

int check_url_port(http_req *req_struct)
{
    int serv_port_num = 80; // the default server port
    char request_content[128];

    // check if http version is 1.1. If so, change to 1.0
    if (strcmp(req_struct->http_version, "HTTP/1.1") == 0)
        strcpy(req_struct->http_version, "HTTP/1.0");
    
    char *str_ptr = strstr(req_struct->http_url, "http://");
    if (str_ptr == NULL) {
        fprintf(stderr, "Uncorrect url.\n");
        return -1;
    }

    str_ptr += 8;
    if ((str_ptr = strstr(str_ptr, ":")) == NULL) {
        // choosing the default port 80
        if (sscanf(str_ptr, "%s", request_content) == 0) {
            fprintf(stderr, "Can't find corresponding request.\n");
            return -1;
        }
    } else {
        str_ptr++;
        if (sscanf(str_ptr, "%d%s", &serv_port_num, request_content) == 0) {
            fprintf(stderr, "Can't find corresponding port number.\n");
            return -1;
        }
    }

    strncpy(req_struct->http_url, request_content, sizeof(request_content));
    return serv_port_num;
    
}