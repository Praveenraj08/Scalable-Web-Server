#include "cs537.h"
#include "request.h"
#include<pthread.h>
// #include<time.h>
#include <sys/time.h>


pthread_mutex_t mutex ;
pthread_mutex_t mutex_buff ;
pthread_cond_t can_fill;
pthread_cond_t fill;
int *request_buffer;
int buffer_size;
int buffer_count=0;
int front=0,rear=0;
char *sch_algo=NULL;



// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc !=5) {
    // fprintf(stderr, "Usage: %s <port> <number_of_threads> <size_of_buffer>\n", argv[0]);
    fprintf(stderr,"Usage: %s <port> <number_of_threads> <size_of_buffer>\n",argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
}



void putfd(int connfd)
{

  request_buffer[rear]=connfd;
  rear=(rear+1) % buffer_size;
  buffer_count++;


}

int get_fd_from_sch_algo()
{
  pthread_mutex_lock(&mutex_buff);

  int fd=1;
  if(strcmp(sch_algo,"FIFO")==0)
  {

    fd=request_buffer[front]; //get()
    front=(front+1) % buffer_size;


  }
  else if(strcmp(sch_algo,"SFNF")==0)
  {

  }
  else if(strcmp(sch_algo,"SFF")==0)
  {

  }


buffer_count--;

pthread_mutex_unlock(&mutex_buff);

 return fd;
}


void consumer()
{


  while(1)
  {
    printf("Thread going to lock : %lu \n",pthread_self() );
    pthread_mutex_lock(&mutex);
    // wait while buffer empty

    while(buffer_count==0)
    pthread_cond_wait(&fill,&mutex); // waiting to be filled
    int fd=get_fd_from_sch_algo();
    pthread_cond_signal(&can_fill);
    pthread_mutex_unlock(&mutex);

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
  sch_algo=argv[4];

  request_buffer=malloc(sizeof(int)*buffer_size);
  int k=0;
  for(k=0;k<buffer_size;k++)
  request_buffer[k]=-1;
  pthread_t thread[number_of_threads];

  pthread_mutex_init(&mutex,NULL);
  pthread_mutex_init(&mutex_buff,NULL);
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

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    printf("conn Fd is : %d\n",connfd );
    printf("buff count : %d  front: %d -- rear: %d \n",buffer_count,front,rear);

    pthread_mutex_lock(&mutex);
    while(buffer_count==buffer_size) //producer wait if buffer is full
      pthread_cond_wait(&can_fill,&mutex);

    putfd(connfd); //put()
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);


      // for(i=0;i<number_of_threads;i++)
    // pthread_join(thread[i],NULL);



  } //while(1)

}
