/*
 * client.c: A very, very primitive HTTP client.
 *
 * To run, try:
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * CS537: For testing your server, you will want to modify this client.
 * For example:
 *
 * You may want to make this multi-threaded so that you can
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs;
 * you may want to get more URIs from the command line
 * or read the list from a file.
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "cs537.h"

/*
 * Send an HTTP request for the specified file
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}

/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];
  int length = 0;
  int n;
  // printf("clientPrint fd %d\n",fd );
  Rio_readinitb(&rio, fd);
printf("1\n");
  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  printf("2\n");
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    printf("3\n");
    n = Rio_readlineb(&rio, buf, MAXBUF);
    printf("4\n");

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
    printf("5\n");
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
  printf("6\n");
  printf("End of clientPrint\n");
}

int main(int argc, char *argv[])
{
  char *host, *filename;
  int port;
  int clientfd;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];

  /* Open a single connection to the specified host and port */
  clientfd = Open_clientfd(host, port);
  printf("Client rcvd fd :%d\n",clientfd );
  printf("gng to clientSend\n");
  clientSend(clientfd, filename);
  printf("gng to clientPrint\n");
  clientPrint(clientfd);
  printf("gng to close clientfd\n");
  Close(clientfd);

  exit(0);
}
