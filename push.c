#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char buf[MAXLINE], *astr;
	int length = atoi(getenv("CONTENT_LENGTH"));
	char *method = getenv("REQUEST_METHOD");
	
	//Rio_readn(STDIN_FILENO,buf, length);
	astr = getenv("QUERY_STRING");
	char *name;
	char *time;
	char *value;
	strcpy(buf, astr);
	
	name = strtok(astr, "&");
	time = strtok(NULL, "&");
	value = strtok(NULL, " ");
	

	name = datacut(name);
	time = timecut(time);
	value = datacut(value);
	
	sprintf(buf,"200 OK\r\n");

	Rio_writen(STDIN_FILENO, buf, strlen(buf));

	fprintf(stderr,"WARRING : name : %s to time = %s and value = %s\n", name, time, value);
	fflush(stderr);
}
