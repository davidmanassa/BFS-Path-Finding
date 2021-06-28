#include "BFS.hpp"

// formar edge
void Graph::add_edge(vector<int> adj[], int src, int dest) {
    adj[src].push_back(dest);
    adj[dest].push_back(src);
}

int matToLst1(int x, int y, int mapSizeY) {
    return (x * mapSizeY) + y;
}

void Graph::createEdges(vector<vector<int>> map, vector<int> adj[]) {
    int mapSizeY = map[0].size();
    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < mapSizeY; j++) {
            if (map[i][j] == 0) {
                
                if (i > 0)
                    if (map[i - 1][j] == 0 || map[i - 1][j] == 3 || map[i - 1][j] == 4) {
                        add_edge(adj, matToLst1(i, j, mapSizeY), matToLst1(i - 1, j, mapSizeY));
                    }
                if (j > 0)
                    if (map[i][j - 1] == 0 || map[i][j - 1] == 3 || map[i][j - 1] == 4) {
                        add_edge(adj, matToLst1(i, j, mapSizeY), matToLst1(i, j - 1, mapSizeY));
                    }
                if (i < mapSizeY - 1)
                    if (map[i + 1][j] == 0 || map[i][j - 1] == 3 || map[i][j - 1] == 4) {
                        add_edge(adj, matToLst1(i, j, mapSizeY), matToLst1(i + 1, j, mapSizeY));
                    }
                if (j < map.size() - 1)
                    if (map[i][j + 1] == 0 || map[i][j - 1] == 3 || map[i][j - 1] == 4) {
                        add_edge(adj, matToLst1(i, j, mapSizeY), matToLst1(i, j + 1, mapSizeY));
                    }


            }
        }
    }
}
 
// BFS que armazena o antecessor de cada nodo e a sua distancia
// antecessor, distancia
bool Graph::BFSb(vector<int> adj[], int src, int dest, int v, int pred[], int dist[]) {
    
    // lista de espera
    list<int> queue;
 
    bool visited[v];
 
    for (int i = 0; i < v; i++) {
        visited[i] = false;
        dist[i] = INT_MAX;
        pred[i] = -1;
    }
 
    // a source é o primeiro a ser visitado
    visited[src] = true;
    dist[src] = 0;
    queue.push_back(src);
 
    // algoritmo BFS 
    while (!queue.empty()) {

        int u = queue.front(); // primeiro da queue
        queue.pop_front();

        for (int i = 0; i < adj[u].size(); i++) { // para as ligações

            if (visited[adj[u][i]] == false) { // visitado ?
                visited[adj[u][i]] = true;

                dist[adj[u][i]] = dist[u] + 1;
                pred[adj[u][i]] = u;

                queue.push_back(adj[u][i]);
 
                // Critério da paragem: quando encontramos o destino
                if (adj[u][i] == dest)
                    return true;
            }
        }
    }
 
    return false;
}

// função para obter o menor caminho
vector<int> Graph::shortestDistance(vector<int> adj[], int s, int dest, int v) {

    int pred[v], dist[v];
    // antecessor[i] -> nodo que chegou a este
    // distancia[i] -> quantos nodos percorreu até chegar a este
 
    if (BFSb(adj, s, dest, v, pred, dist) == false) {
        cout << " Não foi possivel conectar o inicio com o fim " << std::endl;
        return vector<int>();
    }
 
    vector<int> path;

    int crawl = dest;
    path.push_back(crawl);

    while (pred[crawl] != -1) { // inicial
        path.push_back(pred[crawl]);
        crawl = pred[crawl];
    }


    cout << "Custo do caminho mais curto : " << dist[dest] << std::endl;
 
    return path;
}
