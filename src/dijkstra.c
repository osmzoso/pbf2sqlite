
/*
** Implementing a graph as an adjacency list
*/

/* struct node adjacency list */
struct Node {
  int dest;
  struct Node* next;
};

/* struct adjacency list */
struct AdjList {
  struct Node* head;
};

/* struct graph */
struct Graph {
  int num_nodes;
  struct AdjList* array;
};

/* add new node */
struct Node* newAdjListNode(int dest) {
  struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
  newNode->dest = dest;
  newNode->next = NULL;
  return newNode;
}

/* create new graph with V nodes */
struct Graph* createGraph(int V) {
  struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
  V = V+1;
  graph->num_nodes = V;
  /* init adjacency list */
  graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));
  /* set head of all lists to NULL */
  for (int i = 0; i < V; ++i)
    graph->array[i].head = NULL;
  return graph;
}

/* release graph memory */
void destroyGraph(struct Graph* graph) {
  for (int i = 0; i < graph->num_nodes; ++i) {
    struct Node* current = graph->array[i].head;
    while (current != NULL) {
      struct Node* temp = current;
      current = current->next;
      free(temp);
    }
  }
  free(graph->array);
  free(graph);
}

/* add an edge to the graph TODO */
void addEdge(struct Graph* graph, int src, int dest) {
  /* edge from src to dest */
  struct Node* newNode = newAdjListNode(dest);
  newNode->next = graph->array[src].head;
  graph->array[src].head = newNode;

  /* if edge undirected then edge from dest to src TODO */
  newNode = newAdjListNode(src);
  newNode->next = graph->array[dest].head;
  graph->array[dest].head = newNode;
}

/* show graph */
void printGraph(struct Graph* graph) {
  for (int n = 0; n < graph->num_nodes; n++) {
    struct Node* pCrawl = graph->array[n].head;
    printf("\nnode %d: ", n);
    while (pCrawl) {
      printf(" ->%d", pCrawl->dest);
      pCrawl = pCrawl->next;
    }
  }
  printf("\n");
}

