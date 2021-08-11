#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define BLACK 'B'
#define RED 'R'
#define LEFT 0
#define RIGHT 1

#define MAX_CMD_LENGTH 10
#define ADD_GRAPH_CMD 'A'
#define TOPK_CMD 'T'

#define MIN_DIST 0
#define MAX_DIST INT_MAX

typedef struct rb_node {
    char color;
    int graphID;
    int bestPathsLengthSum;
    struct rb_node *left;
    struct rb_node *right;
} rb_node_t;

typedef struct vertex {
    int index;
    int *neighbors;
    int distFromSource;
    struct vertex *prev;
} vertex_t;

typedef struct heap {
    vertex_t *vertexes;
    int size;
} heap_t;

void getParameters();

void compute();

void readGraph();

rb_node_t getLastInRanking();

void skipGraph();

void dijkstra(int graphID);

void printTopKGraphs(rb_node_t node);

void buildHeap(heap_t heap);

void minHeapify(heap_t *heap, int pos);

vertex_t deleteMin(heap_t *heap);

struct vertex getVertexByIndex(heap_t *heap, int index);

void updateRank(int graphID, int costSum);

// d := number of nodes, k := rank length
static int d, k;

static rb_node_t *rank;

static int **currGraph;
static int num_rankedGraphs;

static vertex_t *currVertexes;

int main() {

    getParameters();

    rank = (rb_node_t *) malloc(k * sizeof(rb_node_t));

    currGraph = (int **) malloc(d * sizeof(int *));
    for (int i = 0; i < d; i++) {
        currGraph[i] = (int *) malloc(d * sizeof(int));
    }

    currVertexes = (vertex_t *) malloc(d * sizeof(vertex_t));

    compute();

    return 0;
}

void getParameters() {
    scanf("%d %d", &d, &k);
}

void compute() {
    char cmd[MAX_CMD_LENGTH];
    int graphID = -1;

    while (fscanf(stdin, "%s", cmd) == 1) {
        if (cmd[0] == ADD_GRAPH_CMD) {
            readGraph();
            graphID = graphID + 1;
            dijkstra(graphID);
        } else if (cmd[0] == TOPK_CMD) { //maybe not required. just else branch
            printTopKGraphs(*rank);
        } else {
            printf("invalid cmd\n");
        }
    }

    if (feof(stdin)) {
        return;
    }

}

void printTopKGraphs(rb_node_t node) {
    printf("topk\n");
    /*
    printf("%d ", node.graphID);
    printTopKGraphs(*node.left);
    printTopKGraphs(*node.right);
    return;
     */
}

void dijkstra(int graphID) {
    heap_t *q = (heap_t*) malloc(sizeof(heap_t));
    q->vertexes = currVertexes;
    for (int i = 0; i < d; i++) {
        q->vertexes[i].index = i;

        if (i == 0) {
            q->vertexes[i].distFromSource = MIN_DIST;
        } else {
            q->vertexes[i].distFromSource = MAX_DIST;
        }

        q->vertexes[i].neighbors = currGraph[i];

        q->vertexes[i].prev = NULL;

        q->size = d;
    }
    for (int i = 0; i < floor(d) / 2; i++) {
        minHeapify(q, i);
    }

    int bestPathsLengthSum = 0;

    /*
    for(int i = 0; i < d; i++) {
        printf("vertexID: %d, dist: %d\n", q.vertexes[i].index, q.vertexes[i].distFromSource);
    }
    */

    while (q->size > 0) {
        vertex_t u = deleteMin(q);
        for (int i = 1; i < d; i++) {
            vertex_t neighbor = getVertexByIndex(q, i);
            int alt = u.distFromSource + u.neighbors[i];
            if (alt < neighbor.distFromSource) {
                neighbor.distFromSource = alt;
                neighbor.prev = &u;
                minHeapify(q, neighbor.index);
                //decreasePriority(q, neighbors, alt); maybe done with minHeapify
            }
        }
        printf("vertexID: %d has distance from source of: %d\n", u.index, u.distFromSource);
        bestPathsLengthSum = bestPathsLengthSum + u.distFromSource;
    }

    updateRank(graphID, bestPathsLengthSum);

    //test
    /*
    for (int i = 0; i < d; i++) {
        for (int j = 0; j < d; j++)
            printf("%d ", currGraph[i][j]);
        printf("\n");
    }
     */
}

void updateRank(int graphID, int costSum) {
    printf("adding graphID: %d with cost: %d\n\n\n", graphID, costSum);
}

struct vertex getVertexByIndex(heap_t *heap, int index) {
    for (int i = 0; i < heap->size; i++) {
        if (heap->vertexes[i].index == index) {
            return heap->vertexes[i];
        }
    }
}

vertex_t deleteMin(heap_t *heap) {

    /*
    printf("\n\n-----DELETING MIN-----\n");
    for (int i = 0; i < heap->size; ++i) {
        printf("vertexID: %d, dist: %d\n", heap->vertexes[i].index, heap->vertexes[i].distFromSource);
    }
     */

    vertex_t bestVertex = heap->vertexes[0];

    /*
    printf("currMin index: %d\n", bestVertex.index);
    printf("deleting...\n");
     */

    heap->vertexes[0] = heap->vertexes[heap->size - 1];

    heap->size--;
    minHeapify(heap, 0);

    /*
    for (int i = 0; i < heap->size; ++i) {
        printf("vertexID: %d, dist: %d\n", heap->vertexes[i].index, heap->vertexes[i].distFromSource);
    }
     */

    return bestVertex;
}

void minHeapify(heap_t *heap, int pos) {

    int left = 2 * pos + 1;
    int right = 2 * pos + 2;
    int minPos = pos;

    if (left < heap->size && heap->vertexes[left].distFromSource < heap->vertexes[pos].distFromSource)
        minPos = left;

    if (right < heap->size && heap->vertexes[right].distFromSource < heap->vertexes[minPos].distFromSource)
        minPos = right;

    if (pos != minPos) {
        vertex_t temp = heap->vertexes[pos];
        heap->vertexes[pos] = heap->vertexes[minPos];
        heap->vertexes[minPos] = temp;

        minHeapify(heap, minPos);
    }
}

void readGraph() {
    int i = 0;
    int currPathLength;

    if (num_rankedGraphs >= k) {
        int maxInitPathLength = 0;
        for (int j = 0; j < d; j++) {
            scanf("%d,", &currPathLength);
            currGraph[0][j] = currPathLength;
            if (currPathLength > maxInitPathLength)
                maxInitPathLength = currPathLength;
        }
        if (maxInitPathLength > getLastInRanking().bestPathsLengthSum)
            skipGraph();
        i = 1;
    }

    int val;
    for (; i < d; i++) {
        for (int j = 0; j < d; j++) {
            scanf("%d,", &val);
            //test
            //printf("i: %d    j: %d      length: %d\n", i, j, val);
            currGraph[i][j] = val;
        }
    }
}

void skipGraph() {
    for (int i = 1; i < d; i++)
        fscanf(stdin, "%*[^\n]");
}

rb_node_t getLastInRanking() {
    rb_node_t currNode = *rank;

    while (currNode.left != NULL) {
        currNode = *currNode.left;
    }

    return currNode;
}

