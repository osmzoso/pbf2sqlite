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
    else if( strcmp("node", argv[i])==0 && argc>=i+2 ){
      id = get_arg_int64(argv, i+1);
      if( exec ) show_node(db, id);
      i++;
    } 
    else if( strcmp("way", argv[i])==0 && argc>=i+2 ){
      id = get_arg_int64(argv, i+1);
      if( exec ) show_way(db, id);
      i++;
    } 
    else if( strcmp("relation", argv[i])==0 && argc>=i+2 ){
      id = get_arg_int64(argv, i+1);
      if( exec ) show_relation(db, id);
      i++;
    } 
    else if( strcmp("vaddr", argv[i])==0 && argc>=i+6 ){
      lon1 = get_arg_double(argv, i+1);
      lat1 = get_arg_double(argv, i+2);
      lon2 = get_arg_double(argv, i+3);
      lat2 = get_arg_double(argv, i+4);
      if( exec ) html_map_addr(db, lon1, lat1, lon2, lat2, argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("vgraph", argv[i])==0 && argc>=i+6 ){
      lon1 = get_arg_double(argv, i+1);
      lat1 = get_arg_double(argv, i+2);
      lon2 = get_arg_double(argv, i+3);
      lat2 = get_arg_double(argv, i+4);
      if( exec ) html_map_graph(db, lon1, lat1, lon2, lat2, argv[i+5]);
      i = i + 5;
    } 
    else if( strcmp("sql", argv[i])==0 && argc>=i+2 ){
      if( exec ) exec_sql_stmt(db, argv[i+1]);
      i++;
    } 
    else {
      printf("Incorrect option '%s'\n", argv[i]);
      exit(EXIT_FAILURE);
    };
    i++;
  }
}
