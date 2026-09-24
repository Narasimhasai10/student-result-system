#include <cstdio>
#include <mysql.h>
int main() { MYSQL* conn = mysql_init(NULL); if (conn) { printf("MySQL OK\n"); mysql_close(conn); } return 0; }
