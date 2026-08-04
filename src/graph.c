/*
** TODO: deprecated function, will be replaced by slice_way_nodes()
** Determines all waypoints along an edge
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

/**
 * \brief Determine intermediate nodes of a way
 * TODO check double points
 */
void slice_way_nodes(
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
** TODO: deprecated function, will be replaced by create_subgraph_tables()
** Creates subgraph for a given boundingbox.
** The result is stored in the temp. table 'subgraph'.
*/
int create_subgraph_tables_v1(
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

/**
 * \brief Creates from the table 'graph_edges' the temp. tables 'subgraph' and 'subgraph_nodes' in the database
 *
 * \param boundingbox
 * \param mask_permit
 */
int create_subgraph_tables(sqlite3 *db, const bbox b, const int mask_permit){
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
  sqlite3_bind_double(stmt_subgraph, 5, b.min_lon);
  sqlite3_bind_double(stmt_subgraph, 6, b.max_lon);
  sqlite3_bind_double(stmt_subgraph, 7, b.min_lat);
  sqlite3_bind_double(stmt_subgraph, 8, b.max_lat);
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

/**
 * \brief Find the nearest node in the subgraph
 */
int64_t subgraph_nearest_node(
  sqlite3 *db,
  const double lon,
  const double lat
){
  sqlite3_stmt *stmt;
  double min_dist_node = DBL_MAX;
  int64_t graph_node_no, no;
  double graph_node_lon, graph_node_lat, dist;
  no = -1;
  rc = sqlite3_prepare_v2(db, "SELECT no,lon,lat FROM subgraph_nodes", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    graph_node_no = sqlite3_column_int64(stmt, 0);
    graph_node_lon = sqlite3_column_double(stmt, 1);
    graph_node_lat = sqlite3_column_double(stmt, 2);
    dist = sqrt(pow(lon-graph_node_lon, 2) + pow(lat-graph_node_lat, 2));
    if( dist < min_dist_node ){
      no = graph_node_no;
      min_dist_node = dist;
    }
  }
  sqlite3_finalize(stmt);
  return no;
}

/**
 * \brief Get node_id for a subgraph node no
 */
int64_t subgraph_node_id(
  sqlite3 *db,
  const int64_t no
){
  sqlite3_stmt *stmt;
  int64_t node_id = -1;
  rc = sqlite3_prepare_v2(db, "SELECT node_id FROM subgraph_nodes WHERE no=?", -1, &stmt, NULL);
  if( rc!=SQLITE_OK ) abort_db_error(db, rc);
  sqlite3_bind_int64(stmt, 1, no);
  while( sqlite3_step(stmt)==SQLITE_ROW ){
    node_id = sqlite3_column_int64(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return node_id;
}

