/*
 * clientPost.c: A very, very primitive HTTP client for sensor
 * 
 * To run, prepare config-cp.txt and try: 
 *      ./clientPost
 *
 * Sends one HTTP request to the specified HTTP server.
 * Get the HTTP response.
 */


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include "stems.h"

  char temp[MAXLINE];
int change_tpye_int;
float change_tpye_float;

void clientSend(int fd, char *filename, char *body)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "POST %s HTTP/1.1\n", filename);
  sprintf(buf, "%sHost: %s\n", buf, hostname);
  sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
  sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
  sprintf(buf, "%s%s\n", buf, body);
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
  
  Rio_readinitb(&rio, fd);


  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);

  while (strcmp(buf, "\r\n") && (n > 0)) {
    /* If you want to look for certain HTTP tags... */

    if (sscanf(buf, "Content-Length: %d ", &length) == 1)
      printf("Length = %d\n", length);
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }

}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename,char* time, float value)
{
  int clientfd;
  char msg[MAXLINE];

  
  sprintf(msg,"name=%s&",myname);
  sprintf(msg,"%stime=%s&",msg,temp); //temp check
  sprintf(msg,"%svalue=%f",msg,value);


  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, filename, msg);// send msg use new port
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, char *timestring, float *value)
{
  FILE *fp;
  int i;
  int time_len;

  time_t t;
  time(&t);
  fp = fopen("config-cp", "r");
  if (fp == NULL)
    unix_error("config-cp.txt file does not open.");
  fscanf(fp, "%s", myname);
  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", filename);
  fscanf(fp, "%f", value);
  strcpy(timestring,ctime(&t));
  time_len = strlen(timestring);
  for(i = 0; i < time_len-1; i++)
	temp[i] = timestring[i];
  fclose(fp);
}



int main(void)
{
  char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE];
  int port;
  float value;
  float temper;
  char time[MAXLINE];
  char cmd[MAXLINE];
  char args[MAXLINE];
  char* p=NULL;
  FILE *fp= NULL;
  char cmdlist[10][MAXLINE];
  int start=0;
  int end=0;
  int i=0;
  pid_t pid;

  for(i=0;i<10;i++)
    strcpy(cmdlist[end],"NO_DATA");

  while(1){
	  printf("help :If you want to see commands, type 'help'\n");
	  printf("clientPost# ");
    scanf("%s",cmd);
    strcpy(cmdlist[end],cmd);
    end++;
    end%=10;
    if(end == start){start ++; start%=10;}
    printf("------------------------------------------------------------\n");
    getargs_cp(myname, hostname, &port, filename, &time, &value);
    if(p = strstr(cmd,"<")){ // value0  32
      //parse
      strcpy(args,p+sizeof(char));
      *p=0;
      p=strstr(args,">");
      *p=0;
      //file open
      fp = fopen("config-cp", "w");
      if (fp == NULL)
        unix_error("config-cp.txt file does not open.");
      
      
      //act
      if( !strcmp(cmd, "name") || !strcmp(cmd, "NAME") )
        strcpy(myname,args);
      else if( !strcmp(cmd, "value") || !strcmp(cmd, "VALUE") )
        value= atof(args);
      else
       printf("[%s] is not define\n",cmd);

      //save
      fprintf(fp, "%s\n", myname);
      fprintf(fp, "%s\n", hostname);
      fprintf(fp, "%d\n", port);
      fprintf(fp, "%s\n", filename);
      fprintf(fp, "%f\n", value);
      fclose(fp);

    }
    else{
      if (!strcmp(cmd, "help") || !strcmp(cmd, "HELP")){
        printf("help :list available commands\n");
        printf("name : print current sensor name\n");
	      printf("name<sensor> : change sensor name to <sensor>\n");
        printf("value : print current value of sensor\n");
        printf("value<n> : set sensor value to <n>\n");
        printf("send : send (current sensor name,time,value)to server\n");
        printf("clear : clear screen\n");
        printf("quit : quit the program\n\n");
      }
      else if (!strcmp(cmd, "quit") || !strcmp(cmd, "Quit")){
      	printf("The program will be terminated\n");
        break;
	}
      else if (!strcmp(cmd, "name") || !strcmp(cmd, "NAME"))
        printf("current sensor is : %s\n",myname);
      else if (!strcmp(cmd, "value") || !strcmp(cmd, "VALUE")) 
        printf("current value is : %f\n",value);
      else if (!strcmp(cmd, "send") || !strcmp(cmd, "SEND")){
	//printf("test :: %s, %s, %d, %s, %s %f\n",myname, hostname, port, filename, temp, value);
        userTask(myname, hostname, port, filename, time, value);
	}
      else if (!strcmp(cmd, "clear") || !strcmp(cmd, "CLEAR")) 
        system("clear");
      else if (!strcmp(cmd, "log") || !strcmp(cmd, "LOG")){
        for(i=start;i<10;i++){
          printf("%s\n",cmdlist[i]);
        }
        for(i=0;i<start-1;i++){
          printf("%s\n",cmdlist[i]);
        }
      }
      else
        printf("[%s] is not define\n",cmd);
      printf("------------------------------------------------------------\n\n");
    }
}
  


  return(0);
}
