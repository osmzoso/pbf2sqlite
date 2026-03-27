/*
** Convert and check numeric inputs
*/
int64_t get_arg_int64(char **argv, int i) {
  int64_t value;
  char *endptr;
  errno = 0; /* Reset errno before conversion */
  value = strtoll(argv[i], &endptr, 10);
  /* Check for conversion errors */
  if( errno==ERANGE ) {
    printf("Invalid number: Overflow or underflow occurred\n");
    exit(EXIT_FAILURE);
  }
  if( endptr==argv[i] || *endptr!='\0' ) {
    printf("Invalid number: Not a valid integer string\n");
    exit(EXIT_FAILURE);
  }
  return value;
}

double get_arg_double(char **argv, int i) {
  double value;
  char *endptr;
  value = strtod(argv[i], &endptr);
  /* Check if the whole string was converted */
  if( *endptr!='\0' ) {
    printf("Invalid number: Non-numeric characters '%s'\n", endptr);
    exit(EXIT_FAILURE);
  }
  return value;
}

/*
** Parses the arguments and calls the functions if exec is true
*/
void parse_args(sqlite3 *db, int argc, char **argv, int exec) {
  int i;
  int64_t id;
  double lon1, lat1, lon2, lat2;
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
      id = get_arg_int64(argv, 3);
      if( exec ) show_node(db, id);
      break;
    } 
    else if( strcmp("way", argv[2])==0 && argc==4 ){
      id = get_arg_int64(argv, 3);
      if( exec ) show_way(db, id);
      break;
    } 
    else if( strcmp("relation", argv[2])==0 && argc==4 ){
      id = get_arg_int64(argv, 3);
      if( exec ) show_relation(db, id);
      break;
    } 
    else if( strcmp("vaddr", argv[2])==0 && argc==8 ){
      lon1 = get_arg_double(argv, 3);
      lat1 = get_arg_double(argv, 4);
      lon2 = get_arg_double(argv, 5);
      lat2 = get_arg_double(argv, 6);
      if( exec ) html_map_addr(db, lon1, lat1, lon2, lat2, argv[7]);
      break;
    } 
    else if( strcmp("vgraph", argv[2])==0 && argc==8 ){
      lon1 = get_arg_double(argv, 3);
      lat1 = get_arg_double(argv, 4);
      lon2 = get_arg_double(argv, 5);
      lat2 = get_arg_double(argv, 6);
      if( exec ) html_map_graph(db, lon1, lat1, lon2, lat2, argv[7]);
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
    else if( strcmp("route", argv[2])==0 && argc==9 ){
      lon1 = get_arg_double(argv, 3);
      lat1 = get_arg_double(argv, 4);
      lon2 = get_arg_double(argv, 5);
      lat2 = get_arg_double(argv, 6);
      if( exec ) shortest_way(db, lon1, lat1, lon2, lat2, argv[7], argv[8]);
      break;
    } 
    else {
      printf("Incorrect option '%s'\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}
