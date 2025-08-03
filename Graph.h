#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <queue>
#include <limits>

class Graph {
public:
    void addNode(int id);
    void removeNode(int id);
    void addEdge(int u, int v, float weight);
    float getWeight(int u, int v) const;
    std::vector<int> dijkstra(int start, int end) const;
    std::vector<int> dfsPath(int start, int end) const;

private:
    std::unordered_map<int, std::vector<std::pair<int, float>>> adjList;
};

#endif
