#ifndef __REQUEST_H__

typedef struct
{
  int fd;
  char *method;
  char *uri;
  char *filename;
  char *version;
  char *cgiargs;
  int  Static;
  int  valid;
}sock_msg_t;

void requestHandle(int fd_req,char *method_req,char *uri_req,char *version_req,char *cgiargs_req, int Static_req);
void getFilename(sock_msg_t *);
int requestParseURI(sock_msg_t *);
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


#endif
