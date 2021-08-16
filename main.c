#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_CMD_LENGTH 20
#define ADD_GRAPH_CMD 'A'
#define MAX_DIST_DIGITS 10 //max possible distance is 2^32-1
#define DIST_SEPARATOR ','

#define MIN_DIST 0
#define MAX_DIST INT_MAX

typedef struct vertex {
    int index;
    int *neighbors;
    unsigned int distFromSource;
} vertex_t;

typedef struct vertexHeap {
    vertex_t *vertices;
    int size;
} vertexHeap_t;

typedef struct rankableGraph {
    int id;
    unsigned int distSum;
} rankableGraph_t;

typedef struct rankHeap {
    rankableGraph_t *rank;
    int size;
} rankHeap_t;

void getParameters();

void compute(int **currGraph, vertex_t *currVertices);

void readAndComputeGraph(int **currGraph, int graphID, vertex_t *currVertices);

void skipGraph(char *buffer);

unsigned int dijkstra(int **currGraph, vertex_t *currVertices);

void printTopKGraphIDs();

void minVertexHeapify(vertexHeap_t *heap, int pos);

vertex_t deleteMinVertex(vertexHeap_t *heap);

void updateRank(int graphID, unsigned int shortestPathsSum);

unsigned int getLastRankedDist();

void maxGraphHeapify(int pos);

int parent(int i);

void swapGraphs(rankableGraph_t *x, rankableGraph_t *y);

void swapVertex(vertex_t *x, vertex_t *y);

void decreasePriority(vertexHeap_t *pHeap, int index, unsigned int alt);

static int d, k; // d := number of nodes, k := rank length

static rankHeap_t rankHeap;

static int MAX_FIRST_LINE_CHARS, MAX_GRAPH_LINE_CHARS;

int main() {

    getParameters();

    MAX_GRAPH_LINE_CHARS = d * (MAX_DIST_DIGITS + 1) + 2; //d * (dist + separator) + endOfString + newline

    int currGraph[d][d];
    vertex_t currVertices[d];

    rankableGraph_t rank[k];

    rankHeap.rank = rank;
    rankHeap.size = 0;

    compute((int **) currGraph, currVertices);

    return 0;
}

void getParameters() {

    MAX_FIRST_LINE_CHARS = MAX_DIST_DIGITS + 1 + MAX_DIST_DIGITS + 2; //d + k + separator + end of string and newline

    char firstLine[MAX_FIRST_LINE_CHARS];
    if (fgets(firstLine, MAX_FIRST_LINE_CHARS, stdin))
        ;

    char *remained;
    d = (int) strtol(firstLine, &remained, 10);
    k = (int) strtol(remained, NULL, 10);
}

void compute(int **currGraph, vertex_t* currVertices) {
    char cmd[MAX_CMD_LENGTH];
    int graphID = -1;

    while (fgets(cmd, MAX_CMD_LENGTH, stdin) != NULL) {
        if (cmd[0] == ADD_GRAPH_CMD) {
            graphID++;
            readAndComputeGraph(currGraph, graphID, currVertices);
        } else {
            printTopKGraphIDs(rankHeap);
        }
    }
}

void printTopKGraphIDs() {
    for (int i = 0; i < rankHeap.size; i++) {
        if (i == rankHeap.size - 1)
            printf("%d", rankHeap.rank[i].id);
        else
            printf("%d ", rankHeap.rank[i].id);
    }
    printf("\n");
}

void readAndComputeGraph(int **currGraph, int graphID, vertex_t *currVertices) {

    char line[MAX_GRAPH_LINE_CHARS];

    int *pGraph = (int *) currGraph;

    for (int vertexIndex = 0, t = 0; vertexIndex < d; vertexIndex++) {
        if (fgets(line, MAX_GRAPH_LINE_CHARS, stdin))
            ;

        char *input, *remained;
        input = line;
        for (int vertexNeighborIndex = 0; vertexNeighborIndex < d; vertexNeighborIndex++, t++) {
            *(pGraph + t) = strtol(input, &remained, 10);
            if (remained[0] == DIST_SEPARATOR) {
                input = &remained[1];
            } else {
                input = remained;
            }

            //non necessary check: speeds up the computation on inputs where k is small (more "competitive" TopK)
            //if one of first vertex's distance is already worst than the last ranked, skip the graph.
            if (rankHeap.size >= k && vertexIndex == 0 && vertexNeighborIndex != 0 &&
                    *(pGraph + t) >= getLastRankedDist()) {
                skipGraph(line);
                return;
            }
        }
    }

    //printf("PESO GRAFO[%d]: ", graphID);
    updateRank(graphID, dijkstra(currGraph, currVertices));
}

void skipGraph(char *buffer) {
    for (int i = 1; i < d; i++) {
        if (fgets(buffer, MAX_GRAPH_LINE_CHARS, stdin))
            ;
    }
}

unsigned int dijkstra(int **currGraph, vertex_t *currVertices) {

    int *pGraph = (int*)currGraph;
    vertex_t *pVert = currVertices;

    //create the min priority queue
    vertexHeap_t q;
    q.vertices = pVert;
    for (int i = 0; i < d; i++) {
        q.vertices[i].index = i;
        if (i == 0) {
            q.vertices[i].distFromSource = MIN_DIST;
        } else {
            q.vertices[i].distFromSource = MAX_DIST;
        }
        q.vertices[i].neighbors = pGraph;
        q.size = d;

        pGraph = pGraph + d;
    }

    //sum of the shortest paths from the source to each node of the graph
    unsigned int shortestPathsSum = 0;
    while (q.size > 0) {
        vertex_t u = deleteMinVertex(&q);
        for (int i = 0; i < q.size; i++) {
            vertex_t neighbor = q.vertices[i];
            if (u.neighbors[neighbor.index] != 0) {
                unsigned int alt = u.distFromSource + u.neighbors[neighbor.index];
                if (alt < neighbor.distFromSource) {
                    decreasePriority(&q, i, alt);
                }
            }
        }

        if (u.distFromSource == MAX_DIST) { //u is unreachable
            u.distFromSource = 0;
        }

        shortestPathsSum = shortestPathsSum + u.distFromSource;
    }

    return shortestPathsSum;
}

void updateRank(int graphID, unsigned int shortestPathsSum) {

    rankableGraph_t toAdd;
    toAdd.id = graphID;
    toAdd.distSum = shortestPathsSum;

    if (rankHeap.size >= k) {
        if (shortestPathsSum >= getLastRankedDist()) {
            return;
        }
        rankHeap.rank[0] = toAdd;
        maxGraphHeapify(0);
    } else {
        rankHeap.size++;
        rankHeap.rank[rankHeap.size - 1] = toAdd;
        int i = rankHeap.size - 1;
        while (i > 0 && rankHeap.rank[parent(i)].distSum <= rankHeap.rank[i].distSum) {
            swapGraphs(&rankHeap.rank[parent(i)], &rankHeap.rank[i]);
            i = parent(i);
        }
    }
}

unsigned int getLastRankedDist() {
    if (rankHeap.size == 0)
        return MAX_DIST;
    return rankHeap.rank[0].distSum;
}

void swapGraphs(rankableGraph_t *x, rankableGraph_t *y) {
    rankableGraph_t temp = *x;
    *x = *y;
    *y = temp;
}

void swapVertex(vertex_t *x, vertex_t *y) {
    vertex_t temp = *x;
    *x = *y;
    *y = temp;
}

int parent(int i) {
    if (i % 2 == 0)
        return i / 2 - 1;
    return i / 2;
}

void maxGraphHeapify(int pos) {

    int left = 2 * pos + 1;
    int right = 2 * pos + 2;
    int maxPos = pos;

    if (left < rankHeap.size && rankHeap.rank[left].distSum > rankHeap.rank[pos].distSum) {
        maxPos = left;
    }

    if (right < rankHeap.size && rankHeap.rank[right].distSum > rankHeap.rank[maxPos].distSum) {
        maxPos = right;
    }

    if (maxPos != pos) {
        swapGraphs(&rankHeap.rank[pos], &rankHeap.rank[maxPos]);
        maxGraphHeapify(maxPos);
    }
}

vertex_t deleteMinVertex(vertexHeap_t *heap) {
    vertex_t bestVertex = heap->vertices[0];
    heap->vertices[0] = heap->vertices[heap->size - 1];
    heap->size--;
    minVertexHeapify(heap, 0);
    return bestVertex;
}

void minVertexHeapify(vertexHeap_t *heap, int pos) {

    int left = 2 * pos + 1;
    int right = 2 * pos + 2;
    int minPos = pos;

    if (left < heap->size && heap->vertices[left].distFromSource < heap->vertices[pos].distFromSource)
        minPos = left;

    if (right < heap->size && heap->vertices[right].distFromSource < heap->vertices[minPos].distFromSource)
        minPos = right;

    if (pos != minPos) {
        swapVertex(&heap->vertices[pos], &heap->vertices[minPos]);
        minVertexHeapify(heap, minPos);
    }
}

void decreasePriority(vertexHeap_t *pHeap, int index, unsigned int alt) {
    pHeap->vertices[index].distFromSource = alt;

    while (index > 0 && pHeap->vertices[parent(index)].distFromSource > pHeap->vertices[index].distFromSource) {
        swapVertex(&pHeap->vertices[parent(index)], &pHeap->vertices[index]);
        index = parent(index);
    }
}

