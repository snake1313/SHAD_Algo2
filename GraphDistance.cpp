#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <climits>

using std::vector;

struct Edge {
  int begin;
  int end;
  int length;
  Edge() {
    begin = end = length = -1;
  }
  Edge(int from, int to, int distance) {
    begin = from;
    end = to;
    length = distance;
  }
  Edge Reversed() const {
    return Edge(end, begin, length);
  }
};

class Graph {

  int size;
  vector<vector<Edge>> data;

public:

  static const int NO_WAY = INT_MAX;

  Graph(int graph_size, vector<Edge> input_data) {
    size = graph_size;
    data.resize(size);
    for (auto const& edge : input_data) {
      int begin = edge.begin;
      int end = edge.end;
      data[begin].push_back(edge);
      data[end].push_back(edge.Reversed());
    }
  }

  int ShortestDistance(int start, int finish) {
    std::deque<int> updated_vertices;
    updated_vertices.push_back(start);
    vector<int> distance(size, NO_WAY);
    distance[start] = 0;
    while (!updated_vertices.empty()) {
      int vertex = updated_vertices.front();
      updated_vertices.pop_front();
      for (auto const& edge : data[vertex]) {
        int to = edge.end;
        int length = edge.length;
        if (distance[vertex] + length < distance[to]) {
          if (length == 0) {
            updated_vertices.push_front(to);
          }
          else {
            updated_vertices.push_back(to);
          }
          distance[to] = distance[vertex] + length;
        }
      }
    }
    return distance[finish]; 
  }
};

void SolveTask() {
  int size, edges, start, finish;
  std::cin >> size >> edges >> start >> finish;
  vector<Edge> input_data;
  for (int index = 0; index < edges; ++index) {
    int from, to, distance;
    std::cin >> from >> to >> distance;
    input_data.push_back(Edge(from - 1, to - 1, distance));
  }
  Graph graph(size, input_data);
  int distance = graph.ShortestDistance(start - 1, finish - 1);
  std::cout << (distance != graph.NO_WAY ? distance : -1) << std::endl;
}

int main() {
  std::ios::sync_with_stdio(false);
  SolveTask();
  return 0;
}
