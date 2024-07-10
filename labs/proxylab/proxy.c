#include <stdio.h>
#include "csapp.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct {
    char http_method[16];
    char http_url[256];
    char http_version[16];
    char http_req_header[MAXLINE];
} http_req;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Parse the client request content */
void parse_cli_req(int clientfd);
/* Read all the http request headers */
void read_req_headers(rio_t *rp);
/* Parse the client's uri */
void parse_uri(char *uri, char *version, char *filename, int *server_port);
/* Make connection to server and get http content */
void connect_server(int port_num, char *filename, char *buf, int clientfd);

int main(int argc, char *argv[])
{
    // If the user enter less than 2 arguments, reminds the user to enter
    if (argc != 2) {
        fprintf(stderr, "usage %s <port number>", argv[0]);
        exit(0);
    }

    // Initial variables
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    
    // Receive connection from client
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, &clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);

        // parse client http request
        parse_cli_req(connfd);

    }

    return 0;
}

void parse_cli_req(int clientfd)
{
    char http_buf[MAXLINE], method[128], uri[MAXLINE], version[32];
    char filename[MAXLINE];
    int server_port;
    rio_t rio;

    /* Read client request line and headers */
    Rio_readinitb(&rio, clientfd);
    if (!Rio_readlineb(&rio, http_buf, MAXLINE))
        return;
    printf("%s", http_buf);
    sscanf(http_buf, "%s %s %s", method, uri, version);
    if (strcasecmp("GET", method)) {
        fprintf(stderr, "Not a GET HTTP request\n");
        return;
    }
    read_req_headers(&rio);

    /* Parse client uri */
    parse_uri(uri, version, filename, &server_port);
    connect_server(server_port, filename, http_buf, clientfd);

    Close(clientfd);
    return;
}

void read_req_headers(rio_t *rp)
{
    char header_buf[MAXLINE];

    Rio_readlineb(rp, header_buf, MAXLINE);
    printf("%s", header_buf);
    while (strcmp(header_buf, "\r\n")) {
        Rio_readlineb(rp, header_buf, MAXLINE);
        printf("%s", header_buf);
    }
    return;
}

void parse_uri(char *uri, char *version, char *filename, int *server_port)
{
    char *uri_ptr;
    char *http_uri = "http://";
    char *http_version = "HTTP/1.0";

    /* Change http version to 1.0 */
    if (strncmp(version, http_version, strlen(http_version)))
        strncpy(version, http_version, strlen(http_version));
    
    /* Parse the uri and get the corresponding filename */
    if ((uri = strstr(uri, http_uri)) == NULL) {
        fprintf(stderr, "This isn't a http uri.\n");
        return;
    }
    uri += strlen(http_uri);

    if ((uri_ptr = strstr(uri, ":")) == NULL) {
        // dedicated port not found, choose port 80
        *server_port = 80;
        uri = strstr(uri, "/");
        sscanf(uri, "%s", filename);
    } else {
        // use dedicated port number
        uri_ptr += 1;
        sscanf(uri_ptr, "%d%s", server_port, filename);
    }
    printf("Choosing server by port %d with file name : %s\n\n", 
           *server_port, filename);
    return;
}

void connect_server(int port_num, char *filename, char *buf, int clientfd)
{
    char serv_port[16];
    // char buf[MAXLINE];
    rio_t rio;

    sprintf(serv_port, "%d", port_num);
    int serverfd = Open_clientfd("localhost", serv_port);
    sprintf(buf, "GET %s HTTP/1.0\r\n", filename);
    Rio_writen(serverfd, buf, strlen(buf));
    sprintf(buf, "Host: localhost:%d\r\n", port_num);
    Rio_writen(serverfd, buf, strlen(buf));
    sprintf(buf, "\r\n");
    Rio_writen(serverfd, buf, strlen(buf));

    
    Rio_readinitb(&rio, serverfd);
    int recv_size = 0, total_size = 0;
    while ((recv_size = Rio_readnb(&rio, buf, MAXLINE)) > 0) {
        Rio_writen(clientfd, buf, recv_size);
        total_size += recv_size;
        printf("%s", buf);
    }
    // Rio_readnb(&rio, buf, MAXLINE);
    printf("\n\nSending total size: %d bytes\n", total_size);

    // Disconnect the server
    Close(serverfd);
    return;
}
