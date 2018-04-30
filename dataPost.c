#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "Mysql.h"

//
// This program is intended to help you test your web server.
// 

int main(int argc, char *argv[])
{
	
	int db_temp = db_init();
	char *buf[100];
	char Query[100];
	char *name;
	char *time;
	char *value_name;
	float value;
 	FILE * fp;
	char *astr = "Currently, CGI program is running, but argument passing is not implemented.";
 	int qurey_count = 0;

	int len = atoi(getenv("CONTENT_LENGTH"));
	char* method = getenv("REQUEST_METHOD");
	astr = getenv("QUERY_STRING");
	strcpy(buf, astr);

	//parse
	name = strtok(astr, "&");
	time = strtok(NULL, "&");
	value_name = strtok(NULL, " ");
	

	name = datacut(name);
	time = timecut(time);
	value_name = datacut(value_name);

	mknod("/tmp/MYFIFO",S_IFIFO|0666,0);
	fp = fopen("/tmp/MYFIFO","r+");
	fseek(fp, 0, SEEK_SET);
	fprintf(fp,"%s\n",getenv("CONTENT_LENGTH"));
	fprintf(fp,"%s\n",buf);
	sleep(1);
	fclose(fp);
	
	
	sprintf(Query, "select 1 from information_schema.tables where table_name = '%s' and table_schema ='test';", name); // exsit return 1
	Mysql_query(connect_db,Query);
	res_ptr = mysql_store_result(connect_db);
	while(sqlrow = mysql_fetch_row(res_ptr)){
		qurey_count = atoi(sqlrow[0]);
	}

	if(qurey_count != 1)
	{
		sprintf(Query,"create table %s(Time VARCHAR(20), Data float, Num INT not null auto_increment, primary key(Num), index(Num) )ENGINE = InnoDB default character set utf8 collate utf8_general_ci;", name);
		Mysql_query(connect_db,Query);
	}
	
	sprintf(Query, "INSERT into %s values('%s', '%f', NULL);", name, time, atof(value_name));
	Mysql_query(connect_db,Query);
	update_SensorList(name);

	printf("Method: %s\r\n",method);
	printf("HTTP/1.0 200 OK\r\n");
	printf("Server: My Web Server\r\n");
	printf("Content-Length: %d\r\n", len);
	printf("Content-Type: text/plain\r\n\r\n");
	printf("%s\n",buf);


	fflush(stdout);
	fclose(fp);
	return(0);
}
