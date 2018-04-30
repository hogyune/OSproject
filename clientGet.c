/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"



void save_cmd(char hostname[], int *port, char webaddr[]) {
	FILE *fp;    fp = fopen("config-cg.txt", "w");
	if (fp == NULL)
		unix_error("config-cg.txt file does not open.");
	fprintf(fp, "%s\n", hostname);   fprintf(fp, "%d\n", *port);
	fprintf(fp, "%s\n", webaddr);
	fclose(fp);
}

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

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{
  char hostname[MAXLINE], webaddr[MAXLINE];
  int port;
  char CMD[MAXLINE];
  char args[MAXLINE];
  char count[MAXLINE];
  char* cp=0;
  pid_t pid;


  getargs_cg(hostname, &port, webaddr);
  printf("port : %d \n",port);
  pid = fork();
  if(pid == 0){
	Execve("./pushServer.cgi",NULL,NULL);

  }else{
  	while (1){
		printf("clientGet #");
		scanf("%s", CMD);
		if (!strcmp(CMD, "quit") || !strcmp(CMD, "QUITE") || !strcmp(CMD, "EXIT") || !strcmp(CMD, "exit"))
			break;

		if (cp = strstr(CMD, "<")){// cmd<args><count>       
			strcpy(args, cp + sizeof(char)); // cmd args><count>      
			*cp = 0;             
			cp = 0;
			if (cp = strstr(args, "<")){// if format is cmd args><count>    
				strcpy(count, cp + sizeof(char));       
				*cp = 0;
				cp = 0;        
				cp = strstr(args, ">");
				if (!cp){ printf("Invalid format. check >\n"); continue; }
				*cp = 0;        
				cp = strstr(count, ">");        
				if (!cp){ printf("Invalid format. check >\n"); continue; }
				*cp = 0;
				if (!strcmp(CMD, "get") || !strcmp(CMD, "GET")){
					sprintf(webaddr, "/dataGet.cgi?NAME=%s&N=%s", args, count);
				}
				else{ printf("%s is not define\n", args);       continue; }
			}
			else{//if cmd args>          
				cp = 0;
				cp = strstr(args, ">");         
				if (!cp){ printf("Invalid format. check >\n"); continue; }
				*cp = 0;           
				if (!strcmp(CMD, "get") || !strcmp(CMD, "GET")){ sprintf(webaddr, "/dataGet.cgi?NAME=%s&N=1", args); }
				else if (!strcmp(CMD, "info") || !strcmp(CMD, "INFO")){
					sprintf(webaddr, "/dataGet.cgi?command=INFO&value=%s", args);
				}
				else{ printf("%s is not define\n", args);             continue; }
			}//parse cmd          
		}   
		else if (!strcmp(CMD, "LIST") || !strcmp(CMD, "list"))
			sprintf(webaddr, "/dataGet.cgi?command=LIST");
		else{
			printf("check argvs: list or cmd<args1> or cmd<args1><args2>\n", args);
			continue;
		}
		printf("\n\nwebaddr[%s]\n\n", webaddr);
		save_cmd(hostname, &port, webaddr);
		userTask(hostname, port, webaddr);
	}
  }
  return(0);
}
