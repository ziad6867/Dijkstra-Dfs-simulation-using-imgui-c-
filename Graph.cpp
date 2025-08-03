#include "Graph.h"
#include <unordered_map>
#include <set>
#include <limits>
#include <algorithm>
#include <stack>

void Graph::addNode(int id) {
    if (adjList.find(id) == adjList.end()) {
        adjList[id] = {};
    }
}

void Graph::removeNode(int id) {
    adjList.erase(id);
    for (auto& [node, neighbors] : adjList) {
        neighbors.erase(
            std::remove_if(neighbors.begin(), neighbors.end(),
                [id](const std::pair<int, float>& p) { return p.first == id; }),
            neighbors.end()
        );
    }
}

void Graph::addEdge(int u, int v, float weight) {
    adjList[u].push_back({ v, weight });
    adjList[v].push_back({ u, weight }); // Undirected graph
}

float Graph::getWeight(int u, int v) const {
    if (adjList.count(u)) {
        for (const auto& pair : adjList.at(u)) {
            if (pair.first == v) return pair.second;
        }
    }
    return std::numeric_limits<float>::infinity();
}

std::vector<int> Graph::dijkstra(int start, int end) const {
    std::unordered_map<int, float> dist;
    std::unordered_map<int, int> prev;
    std::set<int> visited;

    for (const auto& [node, _] : adjList) {
        dist[node] = std::numeric_limits<float>::infinity();
    }
    dist[start] = 0;

    auto cmp = [&dist](int a, int b) {
        return dist[a] > dist[b];
        };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> pq(cmp);
    pq.push(start);

    while (!pq.empty()) {
        int u = pq.top();
        pq.pop();

        if (visited.count(u)) continue;
        visited.insert(u);

        for (const auto& [v, weight] : adjList.at(u)) {
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push(v);
            }
        }
    }

    std::vector<int> path;
    if (!prev.count(end) && start != end) return path;

    for (int at = end; at != start; at = prev[at]) {
        path.push_back(at);
        if (!prev.count(at)) return {}; 
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<int> Graph::dfsPath(int start, int end) const {
    std::unordered_map<int, bool> visited;
    std::unordered_map<int, int> parent;
    std::vector<int> stack = { start };

    visited[start] = true;

    while (!stack.empty()) {
        int current = stack.back();
        stack.pop_back();

        if (current == end) break;

        for (const auto& [neighbor, _] : adjList.at(current)) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                parent[neighbor] = current;
                stack.push_back(neighbor);
            }
        }
    }

    std::vector<int> path;
    if (!visited[end]) return path;

    for (int at = end; at != start; at = parent[at])
        path.push_back(at);
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}
