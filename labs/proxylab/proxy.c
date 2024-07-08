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

/* This function receive the connection with a client */
void recv_client_req(char *listen_port);
/* Parse the client request content, if success return 0, else return -1 */
// int parse_cli_req(char *req_str, http_req *req_struct);
void parse_cli_req(int clientfd);
/* Read all the http request headers */
void read_req_headers(rio_t *rp);
/* Parse the client's uri */
void parse_uri(char *uri, char *version, char *filename, int *server_port);
/* Make connection to server and get http content */
void connect_server(int port_num, char *filename, char *buf);
/* Check the url, modify the http version and return the server port number*/
int check_url_port(http_req *req_struct);
/* Concatenate http request string for actual server */
int cat_http_req(char *req_str, unsigned strlen, http_req *req_struct);

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
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, &clientaddr, &clientlen);
    Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // parse client http request
    parse_cli_req(connfd);

    Close(connfd);

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
    connect_server(server_port, filename, http_buf);

    /* Transfer the content from server to client */
    Rio_writen(clientfd, http_buf, strlen(http_buf));
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

void connect_server(int port_num, char *filename, char *buf)
{
    char serv_port[16];
    // char buf[MAXLINE];
    rio_t rio;

    sprintf(serv_port, "%d", port_num);
    int clientfd = Open_clientfd("localhost", serv_port);
    sprintf(buf, "GET %s HTTP/1.0\r\n", filename);
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "Host: localhost:%d\r\n", port_num);
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "\r\n");
    Rio_writen(clientfd, buf, strlen(buf));

    
    Rio_readinitb(&rio, clientfd);
    Rio_readnb(&rio, buf, MAXLINE);
    printf("Received from server:\n%s", buf);

    // Disconnect the server
    Close(clientfd);
    return;
}
// void recv_client_req(char *listen_port)
// {
//     // Initial variables
//     int listenfd, connfd;
//     char hostname[MAXLINE], port[MAXLINE];
//     socklen_t clientlen;
//     struct sockaddr_storage clientaddr;
    
//     // Receive connection from client
//     listenfd = Open_listenfd(listen_port);
//     clientlen = sizeof(struct sockaddr_storage);
//     connfd = Accept(listenfd, &clientaddr, &clientlen);
//     Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
//                 port, MAXLINE, 0);
//     printf("Accepted connection from (%s, %s)\n", hostname, port);

//     size_t n;
//     char http_buf[MAXLINE];
//     rio_t client_rio, server_rio;
//     http_req req_content;
//     char serv_port[8] = {0};

//     Rio_readinitb(&client_rio, connfd);
//     n = Rio_readnb(&client_rio, http_buf, MAXLINE);

//     int parse_result = parse_cli_req(http_buf, &req_content);
//     if (parse_result < 0) {
//         fprintf(stderr, "Request parsing error.\n");
//         exit(0);
//     }

//     int server_port = check_url_port(&req_content);
//     if (server_port < 0) {
//         fprintf(stderr, "Server port error.\n");
//         exit(0);
//     }

//     sprintf(serv_port, "%7d", server_port);
//     printf("The server port is using %d\n", server_port);
//     int client_fd = Open_clientfd("localhost", serv_port);
//     int serv_send_len = cat_http_req(http_buf, MAXLINE, &req_content);
//     Rio_writen(client_fd, http_buf, serv_send_len);
//     printf("Sending : \n%s", http_buf);
//     int serv_recv_len = Rio_readnb(&server_rio, http_buf, MAXLINE);
//     printf("The received from server is %s\n", http_buf);
//     Rio_writen(connfd, http_buf, serv_recv_len);

//     Close(connfd);
//     Close(client_fd);
// }

// int parse_cli_req(char *req_str, http_req *req_struct)
// {
//     char *str_ptr = strstr(req_str, "\r\n");
//     if (str_ptr != NULL) {
//         str_ptr += 2;
//         strncpy(req_struct->http_req_header, 
//                 str_ptr, 
//                 sizeof(req_struct->http_req_header) - 1);
//         req_struct->http_req_header[sizeof(req_struct->http_req_header) - 2] = '\0';

//         if (sscanf(req_str, "%s %s %s", 
//                    req_struct->http_method, 
//                    req_struct->http_url, 
//                    req_struct->http_version) != 3) {
//             fprintf(stderr, "Failed to parse HTTP request line.\n");
//             return -1;
//         }

//         if (strcmp(req_struct->http_method, "GET") != 0)
//             return -1;
//     } else {
//         fprintf(stderr, "Failed to find the end of request line.\n");
//         return -1;
//     }
//     return 0;
// }

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

int cat_http_req(char *req_str, unsigned strlen, http_req *req_struct)
{
    return snprintf(req_str, 
                    strlen, 
                    "GET /home.html %s\r\n%s%s\r\n", 
                    req_struct->http_version, 
                    "Connection: Close\r\n", 
                    "Proxy-Connection: Close\r\n");
                    // req_struct->http_req_header);
}