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
    graph->numVertices = V;

    // Adjazenzliste initialisieren
    graph->array = (struct AdjList*)malloc(V * sizeof(struct AdjList));

    // Kopf aller Listen auf NULL setzen
    for (int i = 0; i < V; ++i)
        graph->array[i].head = NULL;

    return graph;
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
        printf("\n Adjacency list of vertex %d\n head ", v);
        while (pCrawl) {
            printf("-> %d", pCrawl->dest);
            pCrawl = pCrawl->next;
        }
        printf("\n");
    }
}

int main() {
    // Anzahl der Knoten im Graphen vorgegeben
    int V = 5;
    struct Graph* graph = createGraph(V);

    // Kanten hinzufügen
    addEdge(graph, 0, 1);
    addEdge(graph, 0, 4);
    addEdge(graph, 1, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);
    addEdge(graph, 3, 4);

    // Graphen ausgeben
    printGraph(graph);

    // Speicher freigeben
    for (int i = 0; i < V; ++i) {
        struct Node* current = graph->array[i].head;
        while (current != NULL) {
            struct Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(graph->array);
    free(graph);

    return 0;
}

