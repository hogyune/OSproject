#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"

void clientSend(int fd, char *hostname,char *filename, char *body)
{
  char buf[MAXLINE];

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
  Rio_writen(fd, buf, strlen(buf));
}
  
void getargs_pc(char *hostname, int *port, char *filename,int * threshold1,int * threshold2)
{
  FILE *fp;

  fp = fopen("config-pc.txt", "r");
  if (fp == NULL)
    unix_error("config-pc.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%d", threshold1);
  fscanf(fp, "%d", threshold2);

 fclose(fp);
}


int main(void)
{
  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
  int clientfd;
  char length[10];
  int port,threshold1,threshold2;
  char data[MAXLINE];
  char buf[MAXLINE];
  char * name;
  char * time;
  char * value;
  int result;
  FILE * fp;

  getargs_pc(hostname,&port,filename,&threshold1,&threshold2);

  umask(0);
  //result = mknod("/tmp/MYFIFO",S_IFIFO|0666,0);

  while(1){

	while((fp = fopen("/tmp/MYFIFO","r+")) == NULL){}

	fgets(length,sizeof(length),fp);
	fgets(data,sizeof(data),fp);
	fclose(fp);

	strcpy(buf,data);


	name = strtok(data,"&");
	time = strtok(NULL,"&");
	value = strtok(NULL," ");

	name = datacut(name);
	time = timecut(time);
	value = datacut(value);

	if(atoi(value) >  threshold1 && strstr("temperature",name) != 0){
		printf("[PUSH] Temperature - connection\n");
		clientfd = Open_clientfd(hostname, port);		
		clientSend(clientfd, hostname,filename,buf);
		printf("200 OK\n");
		Close(clientfd);
	}
	else if(atoi(value) >  threshold2 && strstr("humidity",name) != 0){
		printf("[PUSH] Humidity - connection\n");
		clientfd = Open_clientfd(hostname, port);		
		clientSend(clientfd, hostname,filename,buf);
		printf("200 OK\n");
		Close(clientfd);
	}

	unlink("/tmp/MYFIFO");

 }

  return(0);

}
