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
/* Concatenate http request string for actual server */
int cat_http_req(char *req_str, unsigned strlen, 
                 unsigned port_num, http_req *req_struct);

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
    char http_buf[MAXLINE];
    rio_t client_rio, server_rio;
    http_req req_content;
    char serv_port[8] = {0};

    Rio_readinitb(&client_rio, connfd);
    n = Rio_readnb(&client_rio, http_buf, MAXLINE);

    int parse_result = parse_cli_req(http_buf, &req_content);
    if (parse_result < 0) {
        fprintf(stderr, "Request parsing error.\n");
        exit(0);
    }

    int server_port = check_url_port(&req_content);
    if (server_port < 0) {
        fprintf(stderr, "Server port error.\n");
        exit(0);
    }

    sprintf(serv_port, "%7d", server_port);
    int client_fd = Open_clientfd("localhost", serv_port);
    int serv_send_len = cat_http_req(http_buf, MAXLINE, serv_port, &req_struct);
    Rio_writen(client_fd, http_buf, serv_send_len);
    int serv_recv_len = Rio_readnb(&server_rio, http_buf, MAXLINE);
    Rio_writen(connfd, http_buf, serv_recv_len);

    Close(connfd);
    Close(client_fd);
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
    int serv_port_num = 80; // default server port
    char request_content[128] = {0};

    // check if HTTP version is 1.1. If so, change to 1.0
    if (strcmp(req_struct->http_version, "HTTP/1.1") == 0)
        strcpy(req_struct->http_version, "HTTP/1.0");
    
    // Verify the URL starts with "http://"
    const char *url_start = "http://";
    char *url_ptr = strstr(req_struct->http_url, url_start);
    if (url_ptr == NULL) {
        fprintf(stderr, "Incorrect url.\n");
        return -1;
    }

    // Move the pointer past "http://"
    url_ptr += strlen(url_start);
    
    // Check for a port number
    char *port_ptr = strstr(url_ptr, ":");
    if (port_ptr != NULL) {
        // Found a port number, parse it
        port_ptr++; // Move past ':'
        if (sscanf(port_ptr, "%d%127s", &serv_port_num, request_content) != 2) {
            fprintf(stderr, "Can't find corresponding port number or request.\n");
            return -1;
        }
    } else {
        // No port number, look for the path directly
        char *path_ptr = strstr(url_ptr, "/");
        if ((path_ptr == NULL) || 
            (sscanf(path_ptr, "%127s", request_content) != 1)) {
            fprintf(stderr, "Can't find corresponding request.\n");
            return -1;
        }
    }

    strncpy(req_struct->http_url, request_content, sizeof(request_content) - 1);
    req_struct->http_url[sizeof(req_struct->http_url) - 1] = '\0';
    return serv_port_num;
}

int cat_http_req(char *req_str, unsigned strlen, 
                 unsigned port_num, http_req *req_struct)
{
    return snprintf(req_str, 
                    strlen, 
                    "GET http://localhost:%d%s\r\n%s\r\n", 
                    port_num, 
                    req_struct->http_version, 
                    req_struct->http_req_header);
}