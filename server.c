#include "cs537.h"
#include "request.h"
#include<pthread.h>

//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port> <number_of_threads> <size_of_buffer>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    int number_of_threads=atoi(argv[2]);
    int size_of_buffer=atoi(argv[3]);
    int request_buffer[size_of_buffer];
    printf("Number of threads is : %d\n",number_of_threads );
    pthread_t thread[number_of_threads];

    getargs(&port, argc, argv);

    //
    // CS537: Create some threads...
    //
    int i=0;
    for(i=0;i<number_of_threads;i++)
    {
      if(pthread_create(&thread[i],NULL,(void *) &requestHandle,&connfd) !=0)
      {
        printf("pthread create error\n");
        exit(1);
      }

    }

    listenfd = Open_listenfd(port);
    printf("Listen Fd is : %d\n",listenfd );
    int buffer_size_itr=1;
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
  printf("conn Fd is : %d\n",connfd );

	
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads
	// do the work.
	//

  pthread_join(thread[0],NULL);
	// requestHandle(connfd);

	Close(connfd);
    }

}
