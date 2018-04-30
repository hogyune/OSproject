#define IP "127.0.0.1"
#define ID "root"
#define PASS "skaeo159357"
#define DB "test"
#define PORT 8080
#define ERR -1

#include "stems.h"
#include <mysql/mysql.h>

MYSQL *connect_db;
MYSQL *conn_ptr;
MYSQL_RES * sql_result;
MYSQL_RES * res_ptr;
MYSQL_ROW *sql_row;
MYSQL_ROW *sqlrow;


int db_init();

void Mysql_query(MYSQL *conndb, char *sql);
void Mysql_select(MYSQL *conndb, int id);
void update_SensorList(char *name);

