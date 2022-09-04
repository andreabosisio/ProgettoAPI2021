# Progetto API 2021
This is the project for the *Prova Finale* of the course _Algoritmi e Principi dell'Informatica_ held at Politecnico di Milano.

It is an Algorithms and Data Structures project where a temporal and memory efficient program has to be implemented in C given a natural language specification of its operation. 


## Assignment 
The program to be implemented is called *GraphRanker* which is a program that keeps track of the top $K$ weighted directed graphs given in input dynamically: after entering the number of nodes of each graph and the value of $K$, the user can enter as many graphs as he want by typing `AggiungiGrafo`.
The execution stops when the user types `TopK`, after the program has printed the ID of the graphs in the ranking. 
The metric of the ranking is the sum of the shortest paths between the node $0$ and all the other nodes of the graph.

Full **description of the assignment** is available [here](/presentazione-progetto-api-2021.pdf) (in italian).

## Solution
Data structures used: *binary heaps* for both managing the dynamic ranking and the implementation of Dijkstra algorithm (used to compute the shortest paths).  
//todo : complexity

## Final Grade 
This project has obtained the following evaluation (by an automatic evaluator): *30/30 cum Laude*.
