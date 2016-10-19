#include "cs537.h"
#include "request.h"
#include<pthread.h>
// #include<time.h>
#include <sys/time.h>


pthread_mutex_t mutex ;
pthread_cond_t can_fill;
pthread_cond_t fill;
int *request_buffer;
int buffer_size;
int buffer_count=0;
int front=0,rear=0;



// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc !=4) {
    // fprintf(stderr, "Usage: %s <port> <number_of_threads> <size_of_buffer>\n", argv[0]);
    fprintf(stderr,"Usage: %s <port> <number_of_threads> <size_of_buffer>\n",argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
}


int getfd()
{

  int pos=front%buffer_size;
  int rv=request_buffer[pos]; //get()
  front++;
  return rv;
}

void putfd(int connfd)
{
  int pos=rear%buffer_size;
  request_buffer[pos]=connfd;
  rear++;

}


void consumer()
{
  printf("inside producer / buffer_count : %d\n",buffer_count );

  while(1)
  {

    pthread_mutex_lock(&mutex);
    // wait while buffer empty

    while(buffer_count==0)
    pthread_cond_wait(&fill,&mutex); // waiting to be filled

    printf("After cond_wait while \n");
    int fd=getfd();
    buffer_count--;
    printf("Consumer got fd : %d\n",fd );
    printf("Gng to signal\n");
    pthread_cond_signal(&can_fill);
    pthread_mutex_unlock(&mutex);

    printf("Fd gng to requestHandle %d\n",fd );
    requestHandle(fd);
    Close(fd);

  }

}

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  getargs(&port, argc, argv);

  int number_of_threads=atoi(argv[2]);
  buffer_size=atoi(argv[3]);

  request_buffer=malloc(sizeof(int)*buffer_size);
  printf("Number of threads is : %d\n",number_of_threads );
  pthread_t thread[number_of_threads];

  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&can_fill,NULL);
  pthread_cond_init(&fill,NULL);


  // CS537: Create some threads...
  int i=0;
  for(i=0;i<number_of_threads;i++)
  {
    if(pthread_create(&thread[i],NULL,(void *) &consumer,NULL) !=0)
    {
      printf("pthread create error\n");
      exit(1);
    }
  }

  listenfd = Open_listenfd(port);
  printf("Listen Fd is : %d\n",listenfd );

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    printf("conn Fd is : %d\n",connfd );

    pthread_mutex_lock(&mutex);
    while(buffer_count==buffer_size) //producer wait if buffer is full
      pthread_cond_wait(&can_fill,&mutex);

    putfd(connfd); //put()
    buffer_count++;
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);


      // for(i=0;i<number_of_threads;i++)
    // pthread_join(thread[i],NULL);



  } //while(1)

}
