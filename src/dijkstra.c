
/*
** Graph data structures
** Terms: A graph is a set of vertices (nodes/points) connected by edges (links/lines)
** Implementation as an adjacency list
*/
struct AdjNode {
  int dest;
  int dist;
  int edge;
  struct AdjNode* next;
};

struct AdjList {
  struct AdjNode* head;
};

struct Graph {
  int num_nodes;
  struct AdjList* array;
};

struct AdjNode* newAdjListNode(int dest, int dist, int edge) {
  struct AdjNode* newNode = (struct AdjNode*)malloc(sizeof(struct AdjNode));
  if(!newNode) abort_oom();
  newNode->dest = dest;
  newNode->dist = dist;
  newNode->edge = edge;
  newNode->next = NULL;
  return newNode;
}

struct Graph* createGraph(int V) {
  struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
  if(!graph) abort_oom();
  V = V+1;
  graph->num_nodes = V;
  /* init adjacency list */
  graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));
  if(!graph->array) abort_oom();
  /* set head of all lists to NULL */
  for (int i = 0; i < V; ++i)
    graph->array[i].head = NULL;
  return graph;
}

void destroyGraph(struct Graph* graph) {
  for (int i = 0; i < graph->num_nodes; ++i) {
    struct AdjNode* current = graph->array[i].head;
    while (current != NULL) {
      struct AdjNode* temp = current;
      current = current->next;
      free(temp);
    }
  }
  free(graph->array);
  free(graph);
}

void addEdge(struct Graph* graph, int src, int dest, int dist, int edge, int dir) {
  struct AdjNode* newNode;
  newNode = newAdjListNode(dest, dist, edge);  /* add edge from src to dest */
  newNode->next = graph->array[src].head;
  graph->array[src].head = newNode;
  if(!dir){
    newNode = newAdjListNode(src, dist, edge);  /* add edge from dest to src */
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
  }
}

void printGraph(struct Graph* graph) {
  printf("Graph (%d nodes):\n", graph->num_nodes);
  for (int n = 0; n < graph->num_nodes; n++) {
    struct AdjNode* pCrawl = graph->array[n].head;
    printf("node %d\n", n);
    while (pCrawl) {
      printf("        -> node %d, dist %d, edge %d\n", pCrawl->dest, pCrawl->dist, pCrawl->edge);
      pCrawl = pCrawl->next;
    }
  }
  printf("\n");
}

/*
** Structures for the Dijkstra Algorithm
*/
struct Dijkstra {
  int d;         /* Total distance to the node */
  int v_node;    /* Predecessor node (shortest path tree) */
  int v_edge;    /* Predecessor edge (shortest path tree) */
  int pos_heap;  /* Contains the position of the node in b[] */
};

/* Public variables */
struct Dijkstra* node;
int *b;          /* Array b[] contains the nodes in the priority queue */
int b_size;      /* Contains the current number of nodes in the priority queue */

/*
** Priority Queue
**
** b_insert() : Insert node in priority queue
** b_remove() : Remove the node with minimal distance from priority queue
** b_relax()  : Reduce the distance, adjust priority queue
**
*/
void downheap(int k) {
  int j, v, v_k;

  v = node[ b[k] ].d;
  v_k = b[k];
  while ( k <= b_size/2 ) {
    j = k + k;
    if ( j < b_size && node[ b[j] ].d > node[ b[j+1] ].d ) j++;
    if ( v <= node[ b[j] ].d ) break;
    b[k] = b[j];
    node[ b[k] ].pos_heap = k;
    k = j;
  }
  b[k] = v_k;
  node[ b[k] ].pos_heap = k;
}

void upheap(int k) {
  int v, v_k;

  v = node[ b[k] ].d;
  v_k = b[k];
  node[ 0 ].d = 0;
  while ( node[ b[k/2] ].d > v ) {
    b[k] = b[k/2];
    node[ b[k] ].pos_heap = k;
    k = k/2;
  }
  b[k] = v_k;
  node[ b[k] ].pos_heap = k;
}

void b_insert(int v) {
  b[++b_size] = v;
  upheap( b_size );
}

int b_remove(void) {
  int v;

  v = b[1];
  b[1] = b[b_size--];
  downheap( 1 );
  node[v].pos_heap = 0;
  return v;
}

void b_relax(int k, int v) {
  if ( node[ k ].d > v ) {
    node[ k ].d = v;
    if ( node[k].pos_heap > 0 ) upheap( node[k].pos_heap );
  }
  if ( node[ k ].d < v ) {
    node[ k ].d = v;
    if ( node[k].pos_heap > 0 ) downheap( node[k].pos_heap );
  }
}

/*
** Dijkstra Algorithm
** https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
*/
void Dijkstra(struct Graph* graph, int start_node, int dest_node) {
  int i, minD=0, minB=0;
  /* Allocate memory */
  node = (struct Dijkstra*) malloc(graph->num_nodes * sizeof(struct Dijkstra));
  if(!node) abort_oom();
  b = (int *) malloc(graph->num_nodes * sizeof(int));
  if(!b) abort_oom();
  /* Initialize distances and heap */
  for (i = 0; i < graph->num_nodes; i++) {
    node[i].d = INT_MAX;
    node[i].v_node = 0;
    node[i].v_edge = 0;
    b[i] = 0;
  }
  b_size = 0;
  /* Insert start node in priority queue */
  b_insert(start_node);
  node[start_node].d = 0;
  /* While priority queue is not empty */
  while( b_size!=0 ){
    /* Remove node u with minimal distance from priority queue */
    minB = b_remove();
    minD = node[minB].d;
    /* If node u is the destination node, the algorithm can be aborted */
    if (minB == dest_node) break;
    /* Get each neighbor v of node u */
    struct AdjNode* pCrawl = graph->array[minB].head;
    while (pCrawl) {
      /* If node v has not yet been visited, then add it to the priority queue */
      if (node[pCrawl->dest].d == INT_MAX) b_insert(pCrawl->dest);
      /* If this path is shorter, then relax */
      if (minD + pCrawl->dist < node[pCrawl->dest].d) {
        /* Enter new distance in the priority queue, adjust priority */
        b_relax(pCrawl->dest, minD + pCrawl->dist );
        /* Saving the predecessor node and edge */
        node[pCrawl->dest].v_node = minB;
        node[pCrawl->dest].v_edge = pCrawl->edge;
      }
      pCrawl = pCrawl->next;
    }

  }
  /* Free memory for the heap */
  free(b);
}

void destroyDijkstra() {
  free(node);
}

