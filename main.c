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
    int bestDistsSum;
    struct rb_node *father;
    struct rb_node *left;
    struct rb_node *right;
} rb_node_t;

typedef struct vertex {
    int index;
    int *neighbors;
    unsigned int distFromSource;
    struct vertex *prev;
} vertex_t;

typedef struct vertexHeap {
    vertex_t *vertexes;
    int size;
} vertexHeap_t;

void getParameters();

void compute();

void readGraph();

rb_node_t *getLastInRanking();

void skipGraph();

void dijkstra(int graphID);

void printTopKGraphs(rb_node_t *n);

void buildHeap(vertexHeap_t heap);

void printHeap(vertexHeap_t *heap);

void minHeapify(vertexHeap_t *heap, int pos);

vertex_t deleteMin(vertexHeap_t *heap);

vertex_t *getVertexByIndex(vertexHeap_t *heap, int index);

void updateRank(int graphID, int bestDistsSum);

void insert_case1(rb_node_t *n);
void insert_case2(rb_node_t *n);
void insert_case3(rb_node_t *n);
void insert_case4(rb_node_t *n);
void insert_case5(rb_node_t *n);

void leftRotate(struct rb_node *n);

void rightRotate(struct rb_node *n);

// d := number of nodes, k := rank length
static int d, k;

static rb_node_t *rank;

static int **currGraph;
static int num_rankedGraphs = 0;

static vertex_t *currVertexes;

int main() {

    getParameters();

    /*rank = (rb_node_t *) malloc(k * sizeof(rb_node_t));
    for (int i = 0; i < k; i++) {
        rank[i].bestDistsSum = -1;
    }
     */

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

    //todo error with EOF
    while (fscanf(stdin, "%s", cmd) != EOF) {
        if (cmd[0] == ADD_GRAPH_CMD) {
            readGraph();
            graphID = graphID + 1;
            dijkstra(graphID);
        } else if (cmd[0] == TOPK_CMD) { //maybe not required. just else branch
            printTopKGraphs(rank);
            printf("\n");
        } else {
            printf("invalid cmd\n");
        }
    }
}

void printTopKGraphs(rb_node_t *n) {
    if (n == NULL){
        return;
    }
    printf("%d ", n->bestDistsSum);
    printTopKGraphs(n->left);
    printTopKGraphs(n->right);
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

        q->vertexes[i].prev = NULL;

        q->size = d;
    }
    for (int i = 0; i < floor(d) / 2; i++) {
        minHeapify(q, i);
    }

    unsigned int bestDistsSum = 0;

    /*
    for(int i = 0; i < d; i++) {
        printf("vertexID: %d, dist: %d\n", q.vertexes[i].index, q.vertexes[i].distFromSource);
    }
    */

    while (q->size > 0) {
        vertex_t u = deleteMin(q);
        for (int i = 0; i < d; i++) {
            if (i != u.index) {
                vertex_t *neighbor = getVertexByIndex(q, i);
                if (neighbor != NULL && u.neighbors[i] != 0) {
                    unsigned int alt = u.distFromSource + u.neighbors[i];
                    if (alt < neighbor->distFromSource) {
                        neighbor->distFromSource = alt;
                        neighbor->prev = &u;
                        //review
                        minHeapify(q, 0);
                        //decreasePriority(q, neighbors, alt); maybe done with minHeapify
                    }
                }
            }
        }
        if (u.distFromSource == MAX_DIST) { //u is unreachable
            u.distFromSource = 0;
        }
        //printf("vertexID: %d has distance from source of: %d\n", u.index, u.distFromSource);
        bestDistsSum = bestDistsSum + u.distFromSource;
    }

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

void updateRank(int graphID, int bestDistsSum) {
    //printf("PESO GRAFO[%d]: %d\n", graphID, bestDistsSum);

    num_rankedGraphs++;

    rb_node_t *toAdd = (rb_node_t *) malloc(sizeof(rb_node_t));
    toAdd->graphID = graphID;
    toAdd->bestDistsSum = bestDistsSum;
    toAdd->color = RED;
    toAdd->father = NULL;
    toAdd->left = NULL;
    toAdd->right = NULL;

    rb_node_t *y = NULL;
    rb_node_t *x = rank;

    while(x != NULL) {
        y = x;
        if (bestDistsSum < x->bestDistsSum) {
            x = x->left;
        } else if (bestDistsSum == x->bestDistsSum) {
            break;
        } else {
            x = x->right;
        }
    }
    toAdd->father = y;
    if(y == NULL) {
        rank = toAdd;
    } else if(toAdd->bestDistsSum < y->bestDistsSum) {
        y->left = toAdd;
    } else {
        y->right = toAdd;
    }

    insert_case1(toAdd);

    if(num_rankedGraphs > k) {
        //todo delete the max-dist node
        //free(getLastInRanking());
        //num_rankedGraphs--;
    }
    //printTopKGraphs(rank);
}

rb_node_t *grandparent(rb_node_t *n) {
    return n->father->father;
}

rb_node_t *uncle(rb_node_t *n) {
    if (n->father == grandparent(n)->left)
        return grandparent(n)->right;
    else
        return grandparent(n)->left;
}

void insert_case1(rb_node_t *n) {
    if (n->father == NULL)
        n->color = BLACK;
    else
        insert_case2(n);
}

void insert_case2(rb_node_t *n) {
    if (n->father->color == BLACK)
        return; /* Tree is still valid */
    else
        insert_case3(n);
}

void insert_case3(rb_node_t *n) {
    if (uncle(n) != NULL && uncle(n)->color == RED) {
        n->father->color = BLACK;
        uncle(n)->color = BLACK;
        grandparent(n)->color = RED;
        insert_case1(grandparent(n));
    }
    else
        insert_case4(n);
}

void insert_case4(rb_node_t *n) {
    if (n == n->father->right && n->father == grandparent(n)->left) {
        leftRotate(n->father);
        n = n->left;
    } else if (n == n->father->left && n->father == grandparent(n)->right) {
        rightRotate(n->father);
        n = n->right;
    }
    insert_case5(n);
}

void rightRotate(struct rb_node *n) {
    rb_node_t *y = n->left;
    n->left = y->right;
    if(y->right != NULL) {
        y->right->father = n;
    }
    y->father = n->father;
    if(n->father == NULL) {
        rank = y;
    } else if (n == n->father->left) {
        n->father->left = y;
    } else {
        n->father->right = y;
    }
    y->right = n;
    n->father = y;
}

void leftRotate(struct rb_node *n) {
    rb_node_t *y = n->right;
    n->right = y->left;
    if(y->left != NULL) {
        y->left->father = n;
    }
    y->father = n->father;
    if(n->father == NULL) {
        rank = y;
    } else if (n == n->father->left) {
        n->father->left = y;
    } else {
        n->father->right = y;
    }
    y->left = n;
    n->father = y;
}

void insert_case5(rb_node_t *n) {
    n->father->color = BLACK;
    grandparent(n)->color = RED;
    if (n == n->father->left && n->father == grandparent(n)->left) {
        rightRotate(grandparent(n));
    } else {
        /* Here, n == n->father->right && n->father == grandparent(n)->right */
        leftRotate(grandparent(n));
    }
}


vertex_t *getVertexByIndex(vertexHeap_t *heap, int index) {
    for (int i = 0; i < heap->size; i++) {
        if (heap->vertexes[i].index == index) {
            return &heap->vertexes[i];
        }
    }
    return NULL;
}

vertex_t deleteMin(vertexHeap_t *heap) {

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

void minHeapify(vertexHeap_t *heap, int pos) {

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
        if (maxInitPathLength > getLastInRanking()->bestDistsSum)
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

rb_node_t *getLastInRanking() {
    rb_node_t *currNode = rank;

    while (currNode->left != NULL) {
        currNode = currNode->left;
    }

    return currNode;
}

void printHeap(vertexHeap_t *heap) {
    for (int i = 0; i < heap->size; i++) {
        printf("v. index: %d has dist: %d\n", heap->vertexes[i].index, heap->vertexes[i].distFromSource);
    }
    printf("------\n\n");
}

