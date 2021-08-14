#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define MAX_CMD_LENGTH 20
#define ADD_GRAPH_CMD 'A'
#define TOPK_CMD 'T'
#define MAX_DIST_DIGITS 10 //max possible distance is 2^32-1
#define DIST_SEPARATOR ','

#define MIN_DIST 0
#define MAX_DIST INT_MAX

typedef struct vertex {
    int index;
    int *neighbors;
    unsigned int distFromSource;
    //struct vertex *prev;
} vertex_t;

typedef struct vertexHeap {
    vertex_t *vertices;
    int size;
} vertexHeap_t;

typedef struct rankableGraph {
    int id;
    int distSum;
} rankableGraph_t;

typedef struct rankHeap {
    rankableGraph_t *rank;
    int size;
} rankHeap_t;

void getParameters();

void compute();

void readGraph();

void skipGraph();

void dijkstra(int graphID);

void printTopKGraphIDs();

//void buildHeap(vertexHeap_t heap);

void printHeap(vertexHeap_t *heap);

void minVertexHeapify(vertexHeap_t *heap, int pos);

vertex_t deleteMinVertex(vertexHeap_t *heap);

void updateRank(int graphID, int bestDistsSum);

int getLastRankedDist();

void maxGraphHeapify(int pos);

int parent(int i);

void swapGraphs(rankableGraph_t *x, rankableGraph_t *y);

void swapVertex(vertex_t* x, vertex_t* y);

void decreasePriority(vertexHeap_t *pHeap, int index, unsigned int alt);

// d := number of nodes, k := rank length
static int d, k;

static rankHeap_t *rankHeap;

static int **currGraph;

static vertex_t *currVertexes;

int main() {

    getParameters();

    rankHeap = (rankHeap_t *) malloc(sizeof(rankHeap_t));
    rankHeap->rank = (rankableGraph_t *) malloc((k) * sizeof(rankableGraph_t));
    rankHeap->size = 0;

    currGraph = (int **) malloc(d * sizeof(int *));
    for (int i = 0; i < d; i++) {
        currGraph[i] = (int *) malloc(d * sizeof(int));
    }

    currVertexes = (vertex_t *) malloc(d * sizeof(vertex_t));

    compute();

    return 0;
}

void getParameters() {
    char firstLine[2 * MAX_DIST_DIGITS + 1 + 2]; //space for d, k, separator, end of string and newline
    if(fgets(firstLine, 2 * MAX_DIST_DIGITS + 1 + 2, stdin))
        ;

    //meglio della scanf: fa passare la lode
    char *remained;
    d = (int) strtol(firstLine, &remained, 10);
    k = (int) strtol(remained, NULL, 10);
}

void compute() {
    char cmd[MAX_CMD_LENGTH];
    int graphID = -1;

    while (fgets(cmd, MAX_CMD_LENGTH, stdin) != NULL) {
        if (cmd[0] == ADD_GRAPH_CMD) {
            readGraph();
            graphID = graphID + 1;
            dijkstra(graphID);
        } else if (cmd[0] == TOPK_CMD) { //maybe not required. just else branch
            printTopKGraphIDs(rankHeap);
        } else {
            printf("invalid cmd\n");
        }
    }
}

void printTopKGraphIDs() {
    for (int i = 0; i < rankHeap->size; i++) {
        if(i == rankHeap->size - 1)
            printf("%d", rankHeap->rank[i].id);
        else
            printf("%d ", rankHeap->rank[i].id);
    }
    printf("\n");
}

void dijkstra(int graphID) {
    vertexHeap_t q;
    q.vertices = currVertexes;
    for (int i = 0; i < d; i++) {
        q.vertices[i].index = i;
        if (i == 0) {
            q.vertices[i].distFromSource = MIN_DIST;
        } else {
            q.vertices[i].distFromSource = MAX_DIST;
        }

        q.vertices[i].neighbors = currGraph[i];

        //q->vertices[i].prev = NULL;

        q.size = d;
    }
    for (int i = floor(d/2); i > 0; i--) {
        minVertexHeapify(&q, i);
    }

    unsigned int bestDistsSum = 0;

    /*
    for(int i = 0; i < d; i++) {
        printf("vertexID: %d, dist: %d\n", q.vertices[i].index, q.vertices[i].distFromSource);
    }
    */

    while (q.size > 0) {
        vertex_t u = deleteMinVertex(&q);
        //printf("min: vertex %d with dist of %d\n", u.index, u.distFromSource);
        for (int i = 0; i < q.size; i++) {
            vertex_t neighbor = q.vertices[i];
            if(u.neighbors[neighbor.index] != 0) {
                unsigned int alt = u.distFromSource + u.neighbors[neighbor.index];
                if (alt < neighbor.distFromSource) {
                    neighbor.distFromSource = alt; //inutile?
                    //neighbor->prev = &u;
                    decreasePriority(&q, i, alt);
                }
            }
        }

        if (u.distFromSource == MAX_DIST) { //u is unreachable
            u.distFromSource = 0;
        }
        //printf("vertexID: %d has distance from source of: %d\n", u.index, u.distFromSource);
        bestDistsSum = bestDistsSum + u.distFromSource;
    }

    //printf("graphId %d has sum of %d\n",graphID, bestDistsSum);
    //printf("%d\n", bestDistsSum);
    updateRank(graphID, bestDistsSum);

    //test
    /*
    for (int i = 0; i < d; i++) {
        for (int j = 0; j < d; j++)
            printf("%d ", currGraph[i][j]);
        printf("\n");
    }
     */
}

void decreasePriority(vertexHeap_t *pHeap, int index, unsigned int alt) {
    /*
    if(alt < pHeap->vertices[index].distFromSource)
        return;
    */
    pHeap->vertices[index].distFromSource = alt;

    //int parent = floor((index-1)/2);
    while (index > 0 && pHeap->vertices[parent(index)].distFromSource > pHeap->vertices[index].distFromSource) {
        swapVertex(&pHeap->vertices[parent(index)], &pHeap->vertices[index]);
        index = parent(index);
    }
}

void updateRank(int graphID, int bestDistsSum) {
    //printf("PESO GRAFO[%d]: %d\n", graphID, bestDistsSum);

    rankableGraph_t toAdd;
    toAdd.id = graphID;
    toAdd.distSum = bestDistsSum;

    if (rankHeap->size >= k) {
        if(bestDistsSum >= getLastRankedDist()) {
            return;
        }
        //free(rankHeap->rank);
        rankHeap->rank[0] = toAdd;
        maxGraphHeapify(0);
    } else {
        rankHeap->size++;
        rankHeap->rank[rankHeap->size - 1] = toAdd;
        int i = rankHeap->size - 1;
        while (i > 0 && rankHeap->rank[parent(i)].distSum <= rankHeap->rank[i].distSum) {
            swapGraphs(&rankHeap->rank[parent(i)], &rankHeap->rank[i]);
            i = parent(i);
        }
    }

    //printTopKGraphs(rank);
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
    if(i%2 == 0)
        return i/2-1;
    return i/2;
}

void maxGraphHeapify(int pos) {

    int left = 2 * pos + 1;
    int right = 2 * pos + 2;
    int maxPos = pos;

    if (left < rankHeap->size && rankHeap->rank[left].distSum > rankHeap->rank[pos].distSum) {
        maxPos = left;
    }

    if(right < rankHeap->size && rankHeap->rank[right].distSum > rankHeap->rank[maxPos].distSum) {
        maxPos = right;
    }

    if(maxPos != pos) {
        swapGraphs(&rankHeap->rank[pos], &rankHeap->rank[maxPos]);
        maxGraphHeapify(maxPos);
    }

}

int getLastRankedDist() {
    if (rankHeap->size == 0) {
        return MAX_DIST;
    }
    return rankHeap->rank[0].distSum;
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

void readGraph() {

    char line[d * (MAX_DIST_DIGITS + 1) + 2]; //d * (dist + separator) + endOfString + newline

    for (int vertexIndex = 0; vertexIndex < d; vertexIndex++) {
        if(fgets(line, d * (MAX_DIST_DIGITS + 1) + 2, stdin)) //d * (dist + separator) + endOfString + newline
            ;

        char *input, *remained;
        input = line;
        for(int vertexNeighborIndex = 0; vertexNeighborIndex < d; vertexNeighborIndex++) {
            currGraph[vertexIndex][vertexNeighborIndex] = strtol(input, &remained, 10);
            if(remained[0] == DIST_SEPARATOR) {
                input = &remained[1];
            } else {
                input = remained;
            }

            //todo this is not necessary. no timing problems
            /*
            //if one of first vertex's distance is already worst than the last ranked, skip the graph.
            if(rankHeap->size >= k && vertexIndex == 0 && currGraph[vertexIndex][vertexNeighborIndex] >= getLastRankedDist()) {
                skipGraph();
                return;
            }
             */
        }
    }
}

void skipGraph() {
    //fixme
    for (int i = 1; i < d; i++)
        if(fscanf(stdin, "%*[^\n]"))
            ;
}


void printHeap(vertexHeap_t *heap) {
    /*
    for (int i = 0; i < heap->size; i++) {
        printf("v. index: %d has dist: %d\n", heap->vertices[i].index, heap->vertices[i].distFromSource);
    }
    printf("------\n\n");
     */

    for (int i = 0; i < heap->size; i++) {
        printf("%d ", heap->vertices[i].distFromSource);
    }

    printf("\n");
}

