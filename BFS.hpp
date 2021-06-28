#ifndef BFS
#define BFS

#include <iostream>
#include <list>
#include <vector>
#include <bits/stdc++.h>
using namespace std;

class Graph {

    public:

        Graph() {};

        void add_edge(vector<int> adj[], int src, int dest);
        void createEdges(vector<vector<int>> map, vector<int> adj[]);

        bool BFSb(vector<int> adj[], int src, int dest, int v, int pred[], int dist[]);
        
        vector<int> shortestDistance(vector<int> adj[], int s, int dest, int v);
};

#endif /* BFS_H */