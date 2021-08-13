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
    vertex_t *vertexes;
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
    if(scanf("%d %d\n", &d, &k))
        ;
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
    vertexHeap_t *q = (vertexHeap_t *) malloc(sizeof(vertexHeap_t));
    q->vertexes = currVertexes;
    for (int i = 0; i < d; i++) {
        q->vertexes[i].index = i;

        if (i == 0) {
            q->vertexes[i].distFromSource = MIN_DIST;
        } else {
            q->vertexes[i].distFromSource = MAX_DIST;
        }

        q->vertexes[i].neighbors = currGraph[i];

        //q->vertexes[i].prev = NULL;

        q->size = d;
    }
    for (int i = floor(d/2); i > 0; i--) {
        minVertexHeapify(q, i);
    }

    unsigned int bestDistsSum = 0;

    /*
    for(int i = 0; i < d; i++) {
        printf("vertexID: %d, dist: %d\n", q.vertexes[i].index, q.vertexes[i].distFromSource);
    }
    */

    while (q->size > 0) {
        vertex_t u = deleteMinVertex(q);
        for (int i = 0; i < q->size; i++) {
            vertex_t neighbor = q->vertexes[i];
            if(u.neighbors[neighbor.index] != 0) {
                unsigned int alt = u.distFromSource + u.neighbors[neighbor.index];
                if (alt < neighbor.distFromSource) {
                    neighbor.distFromSource = alt;
                    //neighbor->prev = &u;
                    decreasePriority(q, i, alt);
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
    if(alt < pHeap->vertexes[index].distFromSource)
        return;
    */
    pHeap->vertexes[index].distFromSource = alt;
    while (index > 0 && pHeap->vertexes[parent(index)].distFromSource > pHeap->vertexes[index].distFromSource) {
        swapVertex(&pHeap->vertexes[parent(index)], &pHeap->vertexes[index]);
        index = parent(index);
    }
}

void updateRank(int graphID, int bestDistsSum) {
    //printf("PESO GRAFO[%d]: %d\n", graphID, bestDistsSum);

    if(bestDistsSum >= getLastRankedDist()) {
        return;
    }

    rankableGraph_t* toAdd = (rankableGraph_t *) malloc(sizeof(rankableGraph_t));
    toAdd->id = graphID;
    toAdd->distSum = bestDistsSum;

    if (rankHeap->size >= k) {
        //free(&rankHeap->rank[0]);
        rankHeap->rank[0] = *toAdd;
        maxGraphHeapify(0);
    } else {
        rankHeap->size++;
        rankHeap->rank[rankHeap->size - 1] = *toAdd;
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
    return floor(i/2);
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
    vertex_t bestVertex = heap->vertexes[0];
    heap->vertexes[0] = heap->vertexes[heap->size - 1];
    heap->size--;
    minVertexHeapify(heap, 0);
    return bestVertex;
}

void minVertexHeapify(vertexHeap_t *heap, int pos) {

    int left = 2 * pos + 1;
    int right = 2 * pos + 2;
    int minPos = pos;

    if (left < heap->size && heap->vertexes[left].distFromSource < heap->vertexes[pos].distFromSource)
        minPos = left;

    if (right < heap->size && heap->vertexes[right].distFromSource < heap->vertexes[minPos].distFromSource)
        minPos = right;

    if (pos != minPos) {
        swapVertex(&heap->vertexes[pos], &heap->vertexes[minPos]);
        minVertexHeapify(heap, minPos);
    }
}

void readGraph() {

    char line[d * (MAX_DIST_DIGITS + 1) + 2]; //d * (dist + separator) + endOfString + newline

    //read first line to read the distances from the source. if one of them is already worst than the last ranked, skip the graph.

    for (int vertexIndex = 0; vertexIndex < d; vertexIndex++) {
        if(fgets(line, d * (MAX_DIST_DIGITS + 1) + 2, stdin)) //d * (dist + separator) + endOfString + newline
            ;
        int i = 0;
        char digits[MAX_DIST_DIGITS];
        for(int vertexNeighborIndex = 0; vertexNeighborIndex < d; vertexNeighborIndex++) {
            int j = 0;
            while(line[i] != DIST_SEPARATOR && line[i] != '\n') {
                digits[j] = line[i];
                i++;
                j++;
            }
            digits[j] = '\0';
            currGraph[vertexIndex][vertexNeighborIndex] = atoi(digits);
            i++;

            //if one of first vertex's distance is already worst than the last ranked, skip the graph.
            /*
            if(rankHeap->size >= k && vertexIndex == 0 && currGraph[vertexIndex][vertexNeighborIndex] >= getLastRankedDist()) {
                skipGraph();
                return;
            }
             */
        }
    }
}

void skipGraph() {
    for (int i = 1; i < d; i++)
        if(fscanf(stdin, "%*[^\n]"))
            ;
}


void printHeap(vertexHeap_t *heap) {
    for (int i = 0; i < heap->size; i++) {
        printf("v. index: %d has dist: %d\n", heap->vertexes[i].index, heap->vertexes[i].distFromSource);
    }
    printf("------\n\n");
}

