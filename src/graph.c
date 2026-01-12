/*
**
*/

void edge_points(
  sqlite3 *db,
  uint64_t way_id,
  uint64_t start_node_id,
  uint64_t end_node_id,
  NodeList *nl
){
  int n;
  sqlite3_stmt *stmt_points;
  n = 0;
  rc = sqlite3_prepare_v2(db,
    " SELECT n.lon,n.lat,n.node_id"
    " FROM way_nodes AS wn"
    " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
    " WHERE wn.way_id=?1"
    "   AND wn.node_order>=(SELECT node_order FROM way_nodes"
    "                       WHERE way_id=?2 AND node_id=?3)"
    "   AND wn.node_order<=(SELECT node_order FROM way_nodes"
    "                       WHERE way_id=?4 AND node_id=?5)"
    " ORDER BY wn.node_order",
     -1, &stmt_points, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt_points, 1, way_id);
  sqlite3_bind_int64(stmt_points, 2, way_id);
  sqlite3_bind_int64(stmt_points, 3, start_node_id);
  sqlite3_bind_int64(stmt_points, 4, way_id);
  sqlite3_bind_int64(stmt_points, 5, end_node_id);
  nodelist_clear(nl);
  while( sqlite3_step(stmt_points)==SQLITE_ROW ){
    nodelist_add(nl, (double)sqlite3_column_double(stmt_points, 0),
                     (double)sqlite3_column_double(stmt_points, 1),
                     (double)sqlite3_column_int64(stmt_points, 2) );
    n++;
  }
  sqlite3_finalize(stmt_points);
  /*
  ** If no nodes were found then search in the opposite direction
  */
  if( n==0 ){
    rc = sqlite3_prepare_v2(db,
      " SELECT n.lon,n.lat,n.node_id"
      " FROM way_nodes AS wn"
      " LEFT JOIN nodes AS n ON wn.node_id=n.node_id"
      " WHERE wn.way_id=?1"
      "   AND wn.node_order>=(SELECT node_order FROM way_nodes"
      "                       WHERE way_id=?2 AND node_id=?3)"
      "   AND wn.node_order<=(SELECT node_order FROM way_nodes"
      "                       WHERE way_id=?4 AND node_id=?5)"
      " ORDER BY wn.node_order DESC  -- nodes in reverse order",
     -1, &stmt_points, NULL);
    if( rc!=SQLITE_OK ) abort_db_error(db, rc);
    sqlite3_bind_int64(stmt_points, 1, way_id);
    sqlite3_bind_int64(stmt_points, 2, way_id);
    sqlite3_bind_int64(stmt_points, 3, end_node_id);
    sqlite3_bind_int64(stmt_points, 4, way_id);
    sqlite3_bind_int64(stmt_points, 5, start_node_id);
    nodelist_clear(nl);
    while( sqlite3_step(stmt_points)==SQLITE_ROW ){
      nodelist_add(nl, (double)sqlite3_column_double(stmt_points, 0),
                       (double)sqlite3_column_double(stmt_points, 1),
                       (double)sqlite3_column_int64(stmt_points, 2) );
      n++;
    }
    sqlite3_finalize(stmt_points);
  }
}

/*
** Creates subgraph for a given boundingbox.
** The result is stored in the temp. table 'subgraph'.
*/
int create_subgraph_tables(
  sqlite3 *db,
  const double lon1,
  const double lat1,
  const double lon2,
  const double lat2,
  const int mask_permit
){
  sqlite3_stmt *stmt_subgraph, *stmt_count;
  int number_of_nodes;
  rc = sqlite3_exec(db, "DROP TABLE IF EXISTS subgraph", NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_prepare_v2(db,
    " CREATE TEMP TABLE subgraph AS"
    " SELECT edge_id,start_node_id,end_node_id,dist,way_id,"
    "        CASE"
    "          WHEN (?1&2=2 AND permit&16=16) OR"
    "               (?2&4=4 AND permit&32=32) THEN 1"
    "          ELSE 0"
    "        END AS directed"
    " FROM graph_edges"
    " WHERE permit & ?3 = ?4 AND"
    "       way_id IN ("
    "                  SELECT way_id FROM rtree_way"
    "                  WHERE max_lon>=?5 AND min_lon<=?6"
    "                    AND max_lat>=?7 AND min_lat<=?8"
    "                 )",
     -1, &stmt_subgraph, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int(stmt_subgraph, 1, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 2, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 3, mask_permit);
  sqlite3_bind_int(stmt_subgraph, 4, mask_permit);
  sqlite3_bind_double(stmt_subgraph, 5, lon1);
  sqlite3_bind_double(stmt_subgraph, 6, lon2);
  sqlite3_bind_double(stmt_subgraph, 7, lat1);
  sqlite3_bind_double(stmt_subgraph, 8, lat2);
  rc = sqlite3_step(stmt_subgraph);
  if( rc==SQLITE_DONE ){
    sqlite3_reset(stmt_subgraph);
  }else{
    abort_db_error(db, rc);
  }
  sqlite3_finalize(stmt_subgraph);
  rc = sqlite3_exec(db,
    " DROP TABLE IF EXISTS subgraph_nodes;"
    " CREATE TEMP TABLE subgraph_nodes ("
    "  no      INTEGER PRIMARY KEY,"
    "  node_id INTEGER,"
    "  lon     REAL,"
    "  lat     REAL"
    " );"
    " INSERT INTO subgraph_nodes (node_id, lon, lat)"
    " SELECT s.node_id,n.lon,n.lat FROM"
    " ("
    "  SELECT start_node_id AS node_id FROM subgraph"
    "  UNION"
    "  SELECT end_node_id AS node_id FROM subgraph"
    " ) AS s"
    " LEFT JOIN nodes AS n ON s.node_id=n.node_id;",
     NULL, NULL, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  /* count nodes */
  number_of_nodes = 0;
  rc = sqlite3_prepare_v2(db,
    "SELECT max(no) FROM subgraph_nodes", -1, &stmt_count, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  rc = sqlite3_step(stmt_count);
  if( rc==SQLITE_ROW ) number_of_nodes = sqlite3_column_int(stmt_count, 0);
  sqlite3_finalize(stmt_count);
  return number_of_nodes;
}

