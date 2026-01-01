/*
** Structures and functions for node lists
*/
typedef struct {
  double lon;
  double lat;
  int64_t node_id;
} Node;

typedef struct {
  Node *node;
  size_t size;      /* number of elements used */
  size_t capacity;  /* allocated elements */
} NodeList;

/* Initialize */
void nodelist_init(NodeList *list) {
  list->size = 0;
  list->capacity = 4;  /* initial capacity */
  list->node = malloc(list->capacity * sizeof(Node));
}

/* Add a node */
void nodelist_add(NodeList *list, double lon, double lat, int64_t node_id) {
  if (list->size == list->capacity) {
    list->capacity *= 2;
    list->node = realloc(list->node, list->capacity * sizeof(Node));
    if (!list->node) abort_oom();
  }
  list->node[list->size++] = (Node){lon, lat, node_id};
}

/* Delete all nodes in the list */
void nodelist_clear(NodeList *list) {
  list->size = 0;
}

/* Free memory */
void nodelist_free(NodeList *list) {
  free(list->node);
  list->node = NULL;
  list->size = list->capacity = 0;
}

/* Show nodelist */
void nodelist_show(NodeList *list) {
  printf("\nNumber of nodes in the list: %zu\n", list->size);
  for (size_t i = 0; i < list->size; i++) {
    printf("(%f, %f, %" PRId64 ")\n", list->node[i].lon, list->node[i].lat, list->node[i].node_id);
  }
}

