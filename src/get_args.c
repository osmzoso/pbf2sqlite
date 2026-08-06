/*
** Parses the arguments and calls the functions if exec is true
*/
void parse_args(sqlite3 *db, int argc, char **argv, int exec) {
  int i;
  int64_t id;
  bbox b;
  i = 2;
  while( i<argc ){
    if( strcmp("read", argv[i])==0 && argc>=i+2 ){
      if( exec ) read_osm_file(db, argv[i+1]);
      i++;
    } 
    else if( strcmp("index", argv[i])==0 ){
      if( exec ) add_index(db);
    }
    else if( strcmp("rtree", argv[i])==0 ){
      if( exec ) add_rtree(db);
    }
    else if( strcmp("addr", argv[i])==0 ){
      if( exec ) add_addr(db);
    }
    else if( strcmp("graph", argv[i])==0 ){
      if( exec ) add_graph(db);
    }
    else if( strcmp("node", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_node(db, id);
      break;
    } 
    else if( strcmp("way", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_way(db, id);
      break;
    } 
    else if( strcmp("relation", argv[2])==0 && argc==4 ){
      id = get_argv_int64(argv, 3);
      if( exec ) show_relation(db, id);
      break;
    } 
    else if( strcmp("vaddr", argv[2])==0 && argc==8 ){
      b.min_lon = get_argv_double(argv, 3);
      b.min_lat = get_argv_double(argv, 4);
      b.max_lon = get_argv_double(argv, 5);
      b.max_lat = get_argv_double(argv, 6);
      if( exec ) html_map_addr(db, b, argv[7]);
      break;
    } 
    else if( strcmp("vgraph", argv[2])==0 && argc==8 ){
      b.min_lon = get_argv_double(argv, 3);
      b.min_lat = get_argv_double(argv, 4);
      b.max_lon = get_argv_double(argv, 5);
      b.max_lat = get_argv_double(argv, 6);
      if( exec ) html_map_graph(db, b, argv[7]);
      break;
    } 
    else if( strcmp("sql", argv[2])==0 && argc==4 ){
      if( exec ) sql_exec_stmt(db, argv[3]);
      break;
    } 
    else if( strcmp("sql", argv[2])==0 && argc==3 ){
      if( exec ) sql_read_stdin(db);
      break;
    } 
    else if( strcmp("route", argv[2])==0 && argc>=9 ){
      if( exec ) route(db, argc, argv);
      break;
    } 
    else {
      printf("Incorrect option '%s'\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}
