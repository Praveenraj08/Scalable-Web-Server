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
#include<pthread.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

/*
 * Send an HTTP request for the specified file
 */
 pthread_mutex_t mutex_1;

 char *host;
 int port,filename_itr=0;


void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  printf("buf strlen %lu\n",strlen(buf) );
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
  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

}
double Time_GetSeconds() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double) ((double)t.tv_sec + (double)t.tv_usec / 1e6);
}
void sender(char **filename)
{
  printf("Inside sender\n");

  int clientfd;
  double t1,t2;
  clientfd = Open_clientfd(host, port);
  printf("Sending file %s -- %d \n",filename[filename_itr],clientfd );

  t1=Time_GetSeconds();

  clientSend(clientfd, filename[filename_itr]);
  clientPrint(clientfd);

  t2=Time_GetSeconds();
  printf("t1-t2 : %lf \n",t2-t1 );



  Close(clientfd);


}

int main(int argc, char *argv[])
{
  pthread_t threads[10];
  pthread_mutex_init(&mutex_1,NULL);
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  // filename = argv[3];
  char *filename[8]={ "output.cgi ", "car.html ",
                  "fiftyfive_file.html ", "fourth_file.html ",
                  "home.html ", "six_files.html "
                  "h.html ", "third_file.html "};
  int i=0;
  int t=100;
  for(i=0;i<t;i++)
  {
    if(pthread_create(&threads[i],NULL,(void *) &sender,&filename) !=0)
    {
      printf("pthread create error\n");
      exit(1);
    }
  }

int j=0;
for(j=0;j<t;j++)
pthread_join(threads[j],NULL);
  /* Open a single connection to the specified host and port */

  printf("sender done \n");
  exit(0);
}
