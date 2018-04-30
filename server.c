#include "stems.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <signal.h>



#define TRUE 1
#define FALSE 0
#define ERR -1


struct msg {
	int confd;
	long time;
	
}typedef msg;

 

struct ThreadPool {
	msg* msgQ;
	pthread_t* ptr;			

	int rear;
	int front;
	int count;
	int msgQSize;
	int thrpSize;
}typedef ThreadPool;

 
void initThreadPool(ThreadPool*p, int thp, int qsize);
int dequeue(ThreadPool* p, msg* rmsg);
int enqueue(ThreadPool* p, msg nmsg);
 
void recvMsg(ThreadPool* p, msg* rmsg);
void sendMsg(ThreadPool* p, msg nmsg);
					

sem_t sem_EMP;
sem_t sem_FULL;
sem_t sem_LOCK;

ThreadPool threadPool;
int listenfd;
void sendKill(){
	int i =0;
	msg nmsg;
	nmsg.confd=ERR;	
	nmsg.time=ERR;
	
	for(i=0;i<threadPool.thrpSize;i++){
		sendMsg(&threadPool, nmsg);
	}
	
	for(i=0;i<threadPool.thrpSize;i++){
		pthread_detach(threadPool.ptr[i]);
	}
	
}


void sig_handler(int sig){
	if(sig==2){
		Close(listenfd);
		sendKill();
		freeThreadPool(&threadPool);
	}
} 



void getargs_ws(int *port)
{
	FILE *fp;
	int pool_size, queue_size;
	if ((fp = fopen("config-ws.txt", "r")) == NULL)
		unix_error("config-ws.txt file does not open.");
	fscanf(fp, "%d", port);
	fscanf(fp,"%d", &pool_size);
	fscanf(fp,"%d", &queue_size);
	printf("pool_size :: %d\n", pool_size);
	printf("queue_size :: %d\n", queue_size);
	//����
	initThreadPool(&threadPool, pool_size, queue_size);
	fclose(fp);
}
 
void* consumer(void* data)
{
	msg rmsg;
	rmsg.confd =0;
	
	char buf[MAXLINE];
	struct sockaddr_in clientaddr;
	int connfd = 0;
	while(1){
		recvMsg(&threadPool, &rmsg);
		printf("i get request \n");
		if(rmsg.confd == ERR){
			printf("i get msg for die\n");
			break;		
		}
		requestHandle(rmsg.confd, rmsg.time);
		printf("I'm done. close bye\n");
		Close(rmsg.confd);
		
	} 
}



int main(void)
{
	int  connfd, port, clientlen;
	struct sockaddr_in clientaddr;
	pid_t  pid = 0;
	msg nmsg;
	initWatch();
	getargs_ws(&port);
	listenfd = Open_listenfd(port);
	
	bootThread(&threadPool);
	signal(SIGINT,(void*)sig_handler);
	pid = fork();
	if (pid == 0) {
		Execve("./pushClient.cgi", NULL, NULL);
	}
	else {
		while (1) {
			clientlen = sizeof(clientaddr);
			connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
			//Setenv("connfd_",cgiargs,1);
			nmsg.confd = connfd;
			nmsg.time = getWatch();			
			sendMsg(&threadPool, nmsg);
		}
	}	
	
	return(0);

}
 

void initThreadPool(ThreadPool*p, int thp, int qsize) {

	p->msgQSize = qsize;
	p->msgQ = (msg*)malloc(sizeof(msg)*qsize);
	
	p->thrpSize = thp;
	p->ptr = (pthread_t*)malloc(sizeof(pthread_t) * (threadPool.thrpSize));
	
	p->rear = 0;
	p->front = 0;

	sem_init(&sem_EMP, 0, 0);
	sem_init(&sem_FULL, 0,p->msgQSize );
	sem_init(&sem_LOCK, 0, 1);

}

 

void bootThread(ThreadPool*p) {

	int i = 0;

	int CkErr= FALSE;
	int thState=FALSE;

	for (i = 0; i<p->thrpSize; i++) {
		CkErr = pthread_create(&(p->ptr[i]), NULL, consumer, &thState);
		if (CkErr != FALSE) {
			printf("THREAD FALSE\n");
		}
	}

}

 

void freeThreadPool(ThreadPool*p) {
	p->msgQ = (msg*)malloc(sizeof(msg)*(p->msgQSize));
	p->ptr = (pthread_t*)malloc(sizeof(pthread_t) * (threadPool.thrpSize));
	free(p->msgQ);
	free(p->ptr);
}

int dequeue(ThreadPool* p, msg* rmsg) {

	int i = 0;

	if (p->count != 0) {
		rmsg->confd = p->msgQ[p->front].confd;
		rmsg->time = p->msgQ[p->front].time;
		
		p->count--;
		p->front++;
		p->front %= p->msgQSize;
		return TRUE;
	}

	else
		return FALSE;
	return ERR;
}

 

int enqueue(ThreadPool* p, msg nmsg) {

	if (p->count < p->msgQSize) {
		p->msgQ[p->rear].confd = nmsg.confd;
		p->msgQ[p->rear].time = nmsg.time;
		
		p->rear++;

		p->rear %= p->msgQSize;
		p->count++;
		return TRUE;
	}
	else {
		return FALSE;
	}
	return ERR;
}

void sendMsg(ThreadPool* p, msg nmsg){

	int ck = FALSE;
	sem_wait(&sem_FULL);
	sem_wait(&sem_LOCK);
	ck = enqueue(p, nmsg);
	sem_post(&sem_LOCK);
	sem_post(&sem_EMP);

	if (ck == FALSE)
		printf("MsgQ is Full\n");
	else if (ck == ERR)
		printf("send ERR\n");
}

 

void recvMsg(ThreadPool* p, msg* rmsg) {

	int ck = FALSE;
	sem_wait(&sem_EMP);
	sem_wait(&sem_LOCK);
	ck = dequeue(p, rmsg);
	sem_post(&sem_LOCK);
	sem_post(&sem_FULL);

	if (ck == FALSE)
		printf("MSG EMP\n");
	else if (ck == ERR)
		printf("recv ERR\n");

}

 
