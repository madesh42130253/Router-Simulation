#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_ROUTERS 10
#define INF INT_MAX

/* ==============================
   1. GRAPH USING ADJACENCY LIST
   ============================== */

typedef struct Edge
{
    int destination;
    int cost;
    struct Edge *next;
} Edge;

typedef struct
{
    Edge *head;
} Router;

Router network[MAX_ROUTERS];

void addConnection(int src, int dest, int cost)
{
    Edge *newEdge = malloc(sizeof(Edge));

    if (newEdge == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    newEdge->destination = dest;
    newEdge->cost = cost;
    newEdge->next = network[src].head;
    network[src].head = newEdge;
}

void addLink(int src, int dest, int cost)
{
    addConnection(src, dest, cost);
    addConnection(dest, src, cost);
}

/* ==============================
   2. MIN HEAP IMPLEMENTATION
   ============================== */

typedef struct
{
    int router;
    int distance;
} HeapNode;

typedef struct
{
    HeapNode nodes[MAX_ROUTERS * MAX_ROUTERS];
    int size;
} MinHeap;

void swap(HeapNode *a, HeapNode *b)
{
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void insertHeap(MinHeap *heap, int router, int distance)
{
    if (heap->size >= MAX_ROUTERS * MAX_ROUTERS)
        return;

    int i = heap->size++;

    heap->nodes[i].router = router;
    heap->nodes[i].distance = distance;

    while (i > 0)
    {
        int parent = (i - 1) / 2;

        if (heap->nodes[parent].distance <=
            heap->nodes[i].distance)
            break;

        swap(&heap->nodes[parent], &heap->nodes[i]);
        i = parent;
    }
}

HeapNode extractMin(MinHeap *heap)
{
    HeapNode minimum = heap->nodes[0];

    heap->nodes[0] = heap->nodes[--heap->size];

    int i = 0;

    while (1)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < heap->size &&
            heap->nodes[left].distance <
            heap->nodes[smallest].distance)
        {
            smallest = left;
        }

        if (right < heap->size &&
            heap->nodes[right].distance <
            heap->nodes[smallest].distance)
        {
            smallest = right;
        }

        if (smallest == i)
            break;

        swap(&heap->nodes[i], &heap->nodes[smallest]);
        i = smallest;
    }

    return minimum;
}

/* ==============================
   3. DIJKSTRA'S ALGORITHM
   ============================== */

void dijkstra(int count, int source,
              int distance[], int parent[])
{
    int visited[MAX_ROUTERS] = {0};

    MinHeap heap;
    heap.size = 0;

    for (int i = 0; i < count; i++)
    {
        distance[i] = INF;
        parent[i] = -1;
    }

    distance[source] = 0;

    insertHeap(&heap, source, 0);

    while (heap.size > 0)
    {
        HeapNode current = extractMin(&heap);

        int u = current.router;

        if (visited[u])
            continue;

        visited[u] = 1;

        Edge *temp = network[u].head;

        while (temp != NULL)
        {
            int v = temp->destination;
            int cost = temp->cost;

            if (!visited[v] &&
                distance[u] != INF &&
                distance[u] + cost < distance[v])
            {
                distance[v] = distance[u] + cost;
                parent[v] = u;

                insertHeap(&heap, v, distance[v]);
            }

            temp = temp->next;
        }
    }
}

/* ==============================
   4. ROUTING TABLE USING LINKED LIST
   ============================== */

typedef struct Route
{
    int destination;
    int nextHop;
    int totalCost;
    struct Route *next;
} Route;

void insertRoute(Route **head,
                 int destination,
                 int nextHop,
                 int totalCost)
{
    Route *newRoute = malloc(sizeof(Route));

    if (newRoute == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    newRoute->destination = destination;
    newRoute->nextHop = nextHop;
    newRoute->totalCost = totalCost;

    newRoute->next = *head;
    *head = newRoute;
}

int findNextHop(int source, int destination, int parent[])
{
    int current = destination;

    if (current == source)
        return source;

    if (parent[current] == -1)
        return -1;

    while (parent[current] != source)
    {
        current = parent[current];

        if (current == -1)
            return -1;
    }

    return current;
}

void generateRoutingTable(int count, int source,
                          int distance[], int parent[])
{
    Route *table = NULL;

    for (int i = 0; i < count; i++)
    {
        if (distance[i] == INF)
            continue;

        int nextHop = findNextHop(source, i, parent);

        insertRoute(&table, i, nextHop, distance[i]);
    }

    printf("\n========== ROUTING TABLE ==========\n");

    printf("Destination\tNext Hop\tCost\n");

    Route *temp = table;

    while (temp != NULL)
    {
        if (temp->destination == source)
        {
            printf("R%d\t\tLOCAL\t\t0\n",
                   temp->destination);
        }
        else
        {
            printf("R%d\t\tR%d\t\t%d\n",
                   temp->destination,
                   temp->nextHop,
                   temp->totalCost);
        }

        temp = temp->next;
    }

    while (table != NULL)
    {
        Route *deleteNode = table;
        table = table->next;
        free(deleteNode);
    }
}

/* ==============================
   5. PACKET FORWARDING
   ============================== */

void forwardPacket(int source, int destination,
                   int parent[], int distance[])
{
    if (distance[destination] == INF)
    {
        printf("\nDestination R%d is unreachable!\n",
               destination);
        return;
    }

    int path[MAX_ROUTERS];
    int length = 0;

    int current = destination;

    while (current != -1)
    {
        path[length++] = current;

        if (current == source)
            break;

        current = parent[current];
    }

    if (path[length - 1] != source)
    {
        printf("No valid route found!\n");
        return;
    }

    printf("\n======= PACKET FORWARDING =======\n");

    for (int i = length - 1; i >= 0; i--)
    {
        printf("Packet received at R%d\n", path[i]);

        if (i > 0)
        {
            printf("R%d forwards packet to R%d\n",
                   path[i], path[i - 1]);
        }
    }

    printf("\nPacket delivered successfully!\n");
    printf("Total routing cost: %d\n",
           distance[destination]);
}

/* ==============================
   6. DISPLAY NETWORK
   ============================== */

void displayNetwork(int count)
{
    printf("\n======= NETWORK TOPOLOGY =======\n");

    for (int i = 0; i < count; i++)
    {
        printf("R%d -> ", i);

        Edge *temp = network[i].head;

        while (temp != NULL)
        {
            printf("R%d(%d) -> ",
                   temp->destination,
                   temp->cost);

            temp = temp->next;
        }

        printf("NULL\n");
    }
}

/* ==============================
   7. FREE GRAPH MEMORY
   ============================== */

void freeNetwork(int count)
{
    for (int i = 0; i < count; i++)
    {
        Edge *temp = network[i].head;

        while (temp != NULL)
        {
            Edge *deleteNode = temp;
            temp = temp->next;
            free(deleteNode);
        }

        network[i].head = NULL;
    }
}

/* ==============================
   8. MAIN FUNCTION
   ============================== */

int main()
{
    int count = 5;

    int distance[MAX_ROUTERS];
    int parent[MAX_ROUTERS];

    int source, destination;

    /* Initialize routers */

    for (int i = 0; i < MAX_ROUTERS; i++)
    {
        network[i].head = NULL;
    }

    /* Create network connections */

    addLink(0, 1, 2);
    addLink(0, 2, 1);
    addLink(1, 2, 3);
    addLink(1, 3, 4);
    addLink(2, 4, 2);
    addLink(3, 4, 1);

    printf("====================================\n");
    printf("       ROUTER SIMULATION SYSTEM     \n");
    printf("====================================\n");

    displayNetwork(count);

    printf("\nAvailable Routers: R0 to R%d\n",
           count - 1);

    printf("\nEnter Source Router: ");

    if (scanf("%d", &source) != 1 ||
        source < 0 || source >= count)
    {
        printf("Invalid source router!\n");
        freeNetwork(count);
        return 1;
    }

    printf("Enter Destination Router: ");

    if (scanf("%d", &destination) != 1 ||
        destination < 0 || destination >= count)
    {
        printf("Invalid destination router!\n");
        freeNetwork(count);
        return 1;
    }

    /* Calculate shortest paths */

    dijkstra(count, source, distance, parent);

    /* Generate routing table */

    generateRoutingTable(count, source,
                         distance, parent);

    /* Display shortest path */

    if (distance[destination] != INF)
    {
        printf("\nShortest Path Cost: %d\n",
               distance[destination]);
    }

    /* Forward packet */

    forwardPacket(source, destination,
                  parent, distance);

    freeNetwork(count);

    return 0;
}