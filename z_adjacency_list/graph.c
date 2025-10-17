#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struktur für einen Knoten in der Adjazenzliste
struct Node {
    int dest;
    struct Node* next;
};

// Struktur für die Adjazenzliste
struct AdjList {
    struct Node* head;
};

// Struktur für den Graphen
struct Graph {
    int num_nodes;
    struct AdjList* array;
};

// Funktion, um einen neuen Knoten zu erstellen
struct Node* newAdjListNode(int dest) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Funktion, um einen Graphen mit V Knoten zu erstellen
struct Graph* createGraph(int V) {
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
  V = V+1;
    graph->num_nodes = V;

    // Adjazenzliste initialisieren
    graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));

    // Kopf aller Listen auf NULL setzen
    for (int i = 0; i < V; ++i)
        graph->array[i].head = NULL;

    return graph;
}

// Funktion, Speicher des Graphen freigeben
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

// Funktion, um eine Kante zum Graphen hinzuzufügen
void addEdge(struct Graph* graph, int src, int dest) {
    // Kante von src nach dest hinzufügen
    struct Node* newNode = newAdjListNode(dest);
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    // Da der Graph ungerichtet ist, Kante von dest nach src hinzufügen
    newNode = newAdjListNode(src);
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
}

// Funktion, um den Graphen auszugeben
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

void create_test_graph() {
  // Anzahl der Knoten im Graphen vorgegeben
  int V = 7;
  printf("Demo graph: 7 nodes\n");
  struct Graph* graph = createGraph(V);

  // Kanten hinzufügen
  addEdge(graph, 1, 2);
  addEdge(graph, 1, 7);
  addEdge(graph, 1, 6);
  addEdge(graph, 2, 7);
  addEdge(graph, 2, 3);
  addEdge(graph, 3, 7);
  addEdge(graph, 3, 4);
  addEdge(graph, 4, 5);
  addEdge(graph, 5, 7);
  addEdge(graph, 5, 6);
  addEdge(graph, 6, 7);

  // Graphen ausgeben
  printGraph(graph);

  // Speicher freigeben
  destroyGraph(graph);
}

void create_rand_graph(int nodes, int edges) {
  int i, n1, n2;
  printf("Random graph: %d nodes, %d edges\n", nodes, edges);
  printf("Create graph...\n");
  struct Graph* graph = createGraph(nodes);
  printf("Create edges...\n");
  for( i=1; i<=edges; i++ ) {
    n1 = (int) ( (float) nodes*rand()/(RAND_MAX+1.0));
    n2 = (int) ( (float) nodes*rand()/(RAND_MAX+1.0));
    addEdge(graph, n1, n2);
  }
  printf("Free memory...\n");
  destroyGraph(graph);
}

#define HELP "Usage: %s <option>\n\n"\
             "Options:\n"\
             "  demo                 Creates a simple test graph\n"\
             "  rand NODES EDGES     Creates a simple test graph\n"

int main(int argc, char **argv) {
  if( argc==1 ){
    printf(HELP, argv[0]);
    return(1);
  }
  if( strcmp("demo", argv[1])==0 && argc==2 ){
    create_test_graph();
  } else if( strcmp("rand", argv[1])==0 && argc==4 ){
    create_rand_graph(atoi(argv[2]), atoi(argv[3]));
  }
  return 0;
}

