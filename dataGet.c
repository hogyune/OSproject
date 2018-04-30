#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "Mysql.h"

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi

char content[MAXLINE];
char query[100];
char temp1[100], temp2[50];
void Mysql_shell(char *cmd);
char *datacut1(char *data);
float Max(char *name);
float Min(char *name);


void htmlReturn(void)
{
  char *buf;
  char *ptr;

  /* Make the response body */
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI test result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
  buf = getenv("QUERY_STRING");
  sprintf(content,"%s<p>Env : %s</p>\r\n", content, buf);
  /*ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\r\n", content, ptr);
    ptr = strsep(&buf, "&");
  }*/
  sprintf(content, "%s</body>\r\n</html>\r\n", content);
  sprintf(content,"%s-------------------------------\r\n",content);
  Mysql_shell(buf);

  /* Generate the HTTP response */
  /*printf("Content-Length: %d\r\n", strlen(content));
  printf("Content-Type: text/html\r\n\r\n");*/
  printf("\r\n");
  printf("\n%s\n\n", content);
  fflush(stdout);
}

void textReturn(void)
{
  //char content[MAXLINE];//global char
  char *buf;
  char *ptr;

  buf = getenv("QUERY_STRING");
  sprintf(content,"%sEnv : %s\n", content, buf);
 /* ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\n", content, ptr);
    ptr = strsep(&buf, "&");
  }*/
  
  /* Generate the HTTP response */
  /*printf("Content-Length: %d\n", strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");*/
  printf("%s", content);
  fflush(stdout);
}

int main(void)
{
  db_init();
  htmlReturn();
  
  char* method = getenv("REQUEST_METHOD");
  char* astr = getenv("QUERY_STRING");


  //textReturn();
  return(0);
}

// shell , db
void Mysql_shell(char *cmd)
{
	char data[10], buf[MAXLINE];
	int count_cmd = 0, count_db, id;
	char *pc;
	char *name;
	int filed, count = 0, i, j;
	float value = 0.0, ave =0.0, sum =0.0, max, min;

	if( (strstr(cmd,"list")) || (strstr(cmd,"LIST")) ) 
	{
		sprintf(query, "select name from sensorlist");
		mysql_query(connect_db, query);
		sql_result = mysql_store_result(connect_db);
		while(sql_row = mysql_fetch_row(sql_result)){
			sprintf(content,"%s%s\r\n",content,sql_row[0]);
		}
	}else if(strstr(cmd,"INFO"))
	{	
		strtok(cmd,"&");
		name = strtok(NULL," ");
		name = datacut1(name);
		strcpy(data,name);

		//data get
		sprintf(query,"select data from %s", data);
		mysql_query(connect_db, query);
		sql_result = mysql_store_result(connect_db);

		while(sql_row = mysql_fetch_row(sql_result))
		{
			value = atof(sql_row[0]);
			sum += value;
			count++;
		}
		ave = sum/count;
		max = Max(data);
		min = Min(data);
	
		//id get
		sprintf(query,"select Id from sensorlist where name = '%s'", data);
		mysql_query(connect_db, query);
		sql_result = mysql_store_result(connect_db);
		while(sql_row = mysql_fetch_row(sql_result))
			id = atoi(sql_row[0]);	
		sprintf(temp1,"ID   MAX    MIN     AVE    count\n");
		sprintf(temp1,"%s %d", temp1, id);
		sprintf(temp1,"%s  %.3lf",temp1, max);
		sprintf(temp1,"%s %.3lf", temp1, min);
		sprintf(temp1,"%s %.3lf", temp1, ave);
		sprintf(temp1,"%s   %d", temp1, count);
		
		sprintf(content,"%s %s",content, temp1);
		sprintf(content,"%s\n",content);
	}else //if(strstr(cmd,"GET"))
	{
		name = strtok(cmd,"&");
		pc = strtok(NULL," ");
		name = datacut1(name);
				
		if((pc = datacut1(pc))!= NULL) count_cmd = atoi(pc);
		
		sprintf(query,"select count from sensorlist where name = '%s'", name);
		mysql_query(connect_db, query);
		sql_result = mysql_store_result(connect_db);
		while(sql_row = mysql_fetch_row(sql_result))
			count_db = atoi(sql_row[0]);

		for(i = count_db; i > count_db - count_cmd; i--){
			sprintf(query,"select time, data from %s where num = '%d'", name, i);
			mysql_query(connect_db, query);
			sql_result = mysql_store_result(connect_db);
			filed = mysql_num_fields(sql_result);
			while(sql_row = mysql_fetch_row(sql_result)){
				for(j = 0; j < filed; j++){
					if(j == 0)
						sprintf(content,"%s time : %s,",content, sql_row[j]);
					else
						sprintf(content,"%s %s : %s",content, name, sql_row[j]);
				}
				sprintf(content,"%s\n",content);
				printf("\n");
			}
		}
	}

	sprintf(buf,"HTTP/1.0 200 OK\r\n");
	Rio_writen(STDIN_FILENO, buf, strlen(buf));
}

float Max(char *name)
{
	float max;
	sprintf(query,"select MAX(data) from %s", name);
	mysql_query(connect_db, query);
	res_ptr = mysql_store_result(connect_db);
	while(sqlrow = mysql_fetch_row(res_ptr))
		max = atof(sqlrow[0]);
	return max;	
}

float Min(char *name)
{
	float min;
	sprintf(query,"select MIN(data) from %s", name);
	mysql_query(connect_db, query);
	res_ptr = mysql_store_result(connect_db);
	while(sqlrow = mysql_fetch_row(res_ptr))
		min = atof(sqlrow[0]);
	return min;
}

char *datacut1(char *data){
	char *temp;
	temp = strtok(data,"=");
	temp = strtok(NULL," ");
	return temp;
}
