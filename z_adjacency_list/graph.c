#include <stdio.h>
#include <stdlib.h>

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
    int numVertices;
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
    graph->numVertices = V;

    // Adjazenzliste initialisieren
    graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));

    // Kopf aller Listen auf NULL setzen
    for (int i = 0; i < V; ++i)
        graph->array[i].head = NULL;

    return graph;
}

// Funktion, Speicher des Graphen freigeben
void destroyGraph(struct Graph* graph) {
  for (int i = 0; i < graph->numVertices; ++i) {
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
    for (int v = 0; v < graph->numVertices; ++v) {
        struct Node* pCrawl = graph->array[v].head;
        printf("\nAdjacency list of vertex %d: head ", v);
        while (pCrawl) {
            printf(" ->%d", pCrawl->dest);
            pCrawl = pCrawl->next;
        }
    }
    printf("\n");
}

void create_test_graph() {
  // Anzahl der Knoten im Graphen vorgegeben
  int V = 8;
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

void create_rand_graph(int nodes, int links) {
  int i, n1, n2;
  printf("Graph: %d nodes, %d links\n", nodes, links);

  struct Graph* graph = createGraph(nodes);

  for( i=1; i<=links; i++ ) {
    n1 = (int) ( (float) nodes*rand()/(RAND_MAX+1.0));
    n2 = (int) ( (float) nodes*rand()/(RAND_MAX+1.0));
    //printf(" edge %d - %d\n", n1, n2);
    addEdge(graph, n1, n2);
  }

  // Graphen ausgeben
  //printGraph(graph);

  // Speicher freigeben
  destroyGraph(graph);
}

int main() {
  create_test_graph();
  //create_rand_graph(1000000, 1000000);

  return 0;
}

