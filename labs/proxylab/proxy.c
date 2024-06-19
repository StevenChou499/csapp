#include <stdio.h>
#include "csapp.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[1024], client_port[1024];

    listenfd = Open_listenfd(argv[1]);

    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, &clientaddr, &clientlen);

    printf("Client connected!!\n");

    size_t n;
    char buf[1024];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    n = Rio_readlineb(&rio, buf, 1024);
    printf("%s", buf);

    char *context = "Waterburbur\nToday is a good day\nToday I am talking about Snoopy!!\r\n\r\n";
    int len = strlen(context);
    // char *ret = "HTTP/1.0 200 OK\r\nMIME-Version: 1.0\r\n\r\n";

    char ret[1024];
    sprintf(ret, "HTTP/1.0 200 OK\r\nMIME-Version: 1.0\r\nServer: Longlong\r\n\
                    Content-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", strlen(context), context);

    Rio_writen(connfd, ret, strlen(ret));

    Close(connfd);

    printf("%s", user_agent_hdr);
    return 0;
}
