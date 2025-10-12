/*
** Execute SQL statement
*/
#include "pbf2sqlite.h"

static int print_row(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    if( i>0 ) printf("|");
    printf("%s", argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

void exec_sql_stmt(sqlite3 *db, const char *sql_stmt){
  char *zErrMsg = 0;
  rc = sqlite3_exec(db, sql_stmt, print_row, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}
