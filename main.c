#include <stdio.h>
#include <stdlib.h>

#define BLACK 'B'
#define RED 'R'
#define LEFT 0
#define RIGHT 1

#define MAX_CMD_LENGTH 10
#define ADD_GRAPH_CMD 'A'
#define TOPK_CMD 'T'

typedef struct rb_node{
    char color;
    int graphID;
    int bestPathsLengthSum;
    struct rb_node *left;
    struct rb_node *right;
} rb_node_t;

void getParameters();

void compute();

void readGraph();

rb_node_t getLastInRanking();

void skipGraph();

void dijkstra();

void printTopKGraphs(rb_node_t node);

// d := number of nodes, k := rank length
static int d, k;

static rb_node_t *rank;

static int **currGraph;
static int num_rankedGraphs;
//static *currVertex;

int main() {

    getParameters();

    rank = (rb_node_t *) malloc(k * sizeof(rb_node_t));

    currGraph = (int **) malloc(k * sizeof(int *));
    for(int i=0; i<k; i++) {
        currGraph[i] = (int *) malloc(k * sizeof(int));
    }
    //currVertex = (int*) malloc(k * sizeof(int));

    compute();

    return 0;
}

void getParameters() {
    scanf("%d %d", &d, &k);
}

void compute() {
    char cmd[MAX_CMD_LENGTH];

    while(fscanf(stdin, "%s", cmd) == 1) {
        if(cmd[0] == ADD_GRAPH_CMD) {
            readGraph();
            dijkstra();
        } else if(cmd[0] == TOPK_CMD) { //maybe not mandatory. just else branch
            printTopKGraphs(*rank);
        } else {
            printf("invalid cmd\n");
        }
    }

    if (feof(stdin))
    {
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

void dijkstra() {
    for(int i=0; i<d; i++) {
        for (int j = 0; j < d; j++)
            printf("%d ", currGraph[i][j]);
        printf("\n");
    }
}

void readGraph() {
    int i = 0;
    int currPathLength;

    if(num_rankedGraphs >= k) {
        int maxInitPathLength = 0;
        for(int j=0; j<d; j++) {
            scanf("%d,", &currPathLength);
            currGraph[0][j] = currPathLength;
            if(currPathLength > maxInitPathLength)
                maxInitPathLength = currPathLength;
        }
        if(maxInitPathLength > getLastInRanking().bestPathsLengthSum)
            skipGraph();
        i = 1;
    }

    int val;
    for(; i<d; i++) {
        for (int j = 0; j < d; j++) {
            scanf("%d,", &val);
            printf("i: %d    j: %d      length: %d\n", i, j, val);
            currGraph[i][j] = val;
        }
    }
}

void skipGraph() {
    for(int i=1; i<d; i++)
        fscanf(stdin, "%*[^\n]");
}

rb_node_t getLastInRanking() {
    rb_node_t currNode = *rank;

    while (currNode.left != NULL) {
        currNode = *currNode.left;
    }

    return currNode;
}

