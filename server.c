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

sock_msg_t *sock_msg;

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

int checkFileExists(sock_msg_t *sock)
{
  // printf("getFilExists() 00 --> ||%s-- %s-- %s||\n",sock->method,sock->uri,sock->version );

//, buf[MAXLINE];
  struct stat sbuf;
  printf("FILENAME : %s\n",sock->filename );
  if (stat(sock->filename, &sbuf) < 0) {
     requestError(sock->fd, sock->filename, "404", "Not found", "CS537 Server could not find this file");
     return 1;
  }

  return 0;
}



int putfd(int connfd)
{
  sock_msg[rear].fd=connfd;
  sock_msg[rear].valid=1;
  getFilename(&sock_msg[rear]);
  // printf("putfd 00 --> ||%s-- %s-- %s||\n",sock_msg[rear].method,sock_msg[rear].uri,sock_msg[rear].version );

  sock_msg[rear].Static=requestParseURI(&sock_msg[rear]);
  // printf("putfd 01 --> ||%s-- %s-- %s||\n",sock_msg[rear].method,sock_msg[rear].uri,sock_msg[rear].version );
  // printf("STATIC is %d\n",sock_msg[rear].Static );
 if( checkFileExists(&sock_msg[rear]) ==1)
 {
   printf(" File does not exist\n");
   return 1;
 }
 // printf("Socketmsg --> ||%s-- %s-- %s||\n",sock_msg[rear].method,sock_msg[rear].uri,sock_msg[rear].version );
  rear=(rear+1) % buffer_size;
  buffer_count++;
  return 0;
}

int sfnf()
{
  int minFD_pos;
  int minlen=10000,temp=0;
  printf("BUFFER COUNT : %d\n",buffer_count );
  while(temp<buffer_size)
  {
    if( sock_msg[temp].valid==1)
    {
      if(strlen (sock_msg[temp].filename) <= minlen)
      {
        minlen=strlen (sock_msg[temp].filename);
        minFD_pos=temp;
      }
    }


    temp++;
  }


  return minFD_pos;
}

int sff()
{
  int minFD_pos;
  int minlen=10000,temp=0;
  struct stat sbuf;
printf("BUFFER COUNT : %d\n",buffer_count );
  while(temp<buffer_size)
  {
    if( sock_msg[temp].valid==1)
    {
      stat(sock_msg[temp].filename,&sbuf);
      if(sbuf.st_size <= minlen)
      {
        minlen=strlen (sock_msg[temp].filename);
        minFD_pos=temp;
      }
    }


    temp++;
  }


  return minFD_pos;
}


int get_fd_from_sch_algo()
{
  // pthread_mutex_lock(&mutex_buff);

  int pos;
  if(strcmp(sch_algo,"FIFO")==0)
  {
    // fd=sock_msg[front];
    pos=front;
    front=(front+1) % buffer_size;
  }
  else if(strcmp(sch_algo,"SFNF")==0)
  {
    pos=sfnf();
  }
  else if(strcmp(sch_algo,"SFF")==0)
  {
    pos=sff();
  }

sock_msg[pos].valid=-1;
buffer_count--;

// pthread_mutex_unlock(&mutex_buff);

 return pos;
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
    sleep(5);
    int pos=get_fd_from_sch_algo();
    int fd=sock_msg[pos].fd;
    // char *uri=sock_msg[rear].uri;
    char *filename=sock_msg[pos].filename;
    char *version=sock_msg[pos].version;
    char *method=sock_msg[pos].method;
    char *cgiargs=sock_msg[pos].cgiargs;
    int Static=sock_msg[pos].Static;

    pthread_cond_signal(&can_fill);
    pthread_mutex_unlock(&mutex);

    requestHandle(fd,method,filename,version,cgiargs,Static);
    Close(fd);

  }

}

int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  getargs(&port, argc, argv);

  int number_of_threads=atoi(argv[2]);
  pthread_t thread[number_of_threads];
  buffer_size=atoi(argv[3]);
  sch_algo=argv[4];

  sock_msg=malloc(sizeof(sock_msg_t)*buffer_size);


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

    if(putfd(connfd)==0)
    {
      pthread_cond_signal(&fill);
      pthread_mutex_unlock(&mutex);
    } //put()


  } //while(1)

}
