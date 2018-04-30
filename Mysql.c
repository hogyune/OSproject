#include "Mysql.h"

void Mysql_query(MYSQL *conndb, char *sql)
{
	if(0 != mysql_query(conndb, sql))
	{
		printf("DB connect ERROR\n");
		fflush(stdout);
		exit(0);
	}
}


int db_init()
{
	connect_db = mysql_init(NULL);

	if(!connect_db){
		printf("ERROR\n");
		exit(0);
	}

	
	if(connect_db = mysql_real_connect(connect_db, IP, ID, PASS, DB, 0, NULL, 0))
	{
		printf("DB connect success\n");
	}else
	{
		printf("DB Connect ERROR\n");
		fflush(stdout);
		exit(0);

	}
	return 0;
}

void update_SensorList(char *name)
{
	int count = 0;
	char Query[100];
	int qurey_count = 0;

	sprintf(Query, "select exists (select *from sensorlist where name = '%s') as success;", name);
	Mysql_query(connect_db,Query);
	res_ptr = mysql_store_result(connect_db);
	while(sqlrow = mysql_fetch_row(res_ptr)){
		qurey_count = atoi(sqlrow[0]);
	}

	if(qurey_count != 1)
	{
		sprintf(Query,"insert into sensorlist values('%s', null, 0);", name);
		Mysql_query(connect_db,Query);
	}

	sprintf(Query, "Select Num from %s", name);

	Mysql_query(connect_db, Query);

	sql_result = mysql_store_result(connect_db);

	while(sql_row = mysql_fetch_row(sql_result)){
		count++;
	}
	sprintf(Query, "update sensorlist set COUNT = %d where Name = '%s' and COUNT = %d", count, name,count - 1);

	mysql_query(connect_db, Query);
	mysql_free_result(sql_result);
	mysql_close(connect_db);
}

