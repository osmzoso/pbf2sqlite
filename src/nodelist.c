/**
 * \file nodelist.c
 * \brief Contains functions for creating node lists
 */

/**
 * \brief Defines the struct of a node in the node list
 */
typedef struct {
  double lon;
  double lat;
  int64_t node_id;
} Node;

/**
 * \brief Defines the struct of a node list
 */
typedef struct {
  Node *node;
  size_t size;      /* number of elements used */
  size_t capacity;  /* allocated elements */
} NodeList;

/**
 * \brief Initialize a node list
 */
void initNodelist(NodeList *list){
  list->size = 0;
  list->capacity = 4;  /* initial capacity */
  list->node = malloc(list->capacity * sizeof(Node));
  if (!list->node) abort_msg("Out of memory");
}

/**
 * \brief Adds a node to a node list
 */
void addNodelist(NodeList *list, double lon, double lat, int64_t node_id){
  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->node = realloc(list->node, list->capacity * sizeof(Node));
    if (!list->node) abort_msg("Out of memory");
  }
  list->node[list->size++] = (Node){lon, lat, node_id};
}

/**
 * \brief Marks a node list as empty
 */
void clearNodelist(NodeList *list){
  list->size = 0;
}

/**
 * \brief Free the memory of a node list
 */
void freeNodelist(NodeList *list){
  free(list->node);
  list->node = NULL;
  list->size = list->capacity = 0;
}

/**
 * \brief Displays the node list (for testing purposes)
 */
void printNodelist(NodeList *list){
  printf("\nNumber of nodes in the list: %zu\n", list->size);
  for (size_t i = 0; i < list->size; i++) {
    printf("(%f, %f, %" PRId64 ")\n", list->node[i].lon, list->node[i].lat, list->node[i].node_id);
  }
}
