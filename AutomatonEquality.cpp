#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <map>
#include <deque>

using std::vector;
using std::deque;
using std::cin;

struct Transition {
  int from;
  char symbol;
  int to;
};

class Automaton {

  int states_count;
  int alphabet_size;
  int start_index;
  vector <bool> is_terminal;
  vector <vector<int> > transitions;

  static int Position (int row, int column, int width) {
    return row * width + column;
  }

  vector <vector <int> > Create_graph (const Automaton& other) const {
    int big_size = states_count * other.states_count;
    vector <vector <int> > graph(big_size, vector <int>());
    for (int first_state = 0; first_state < states_count; ++first_state) {
      for (int second_state = 0; second_state < other.states_count; ++second_state) {
        int from = Position(first_state, second_state, other.states_count);
        for (int edge = 0; edge < alphabet_size; ++edge) {
          int to_in_first = transitions[first_state][edge];
          int to_in_second = other.transitions[second_state][edge];
          int to = Position(to_in_first, to_in_second, other.states_count);
          graph[to].push_back(from);
        }
      }
    }
    return graph;
  }

  deque <int> Distinguishable(const Automaton& other) const {
    deque <int> answer;
    for (int first_state = 0; first_state < states_count; ++first_state) {
      for (int second_state = 0; second_state < other.states_count; ++second_state) {
        if (is_terminal[first_state] ^ other.is_terminal[second_state]) {
          answer.push_back(Position(first_state, second_state, other.states_count));
        }
      }
    }
    return answer;
  }

public:

  Automaton() {
    states_count = alphabet_size = start_index = -1;
  }

  Automaton (int size, int alphabet, int start, 
             vector <int>& exits, vector <Transition>& links) {
    states_count = size;
    alphabet_size = alphabet;
    start_index = start;
    is_terminal.resize(states_count, false);
    for (int index = 0; index < exits.size(); ++index) {
      is_terminal[exits[index]] = true;
    }
    transitions.resize(states_count, vector <int> (alphabet_size, -1));
    for (int index = 0; index < links.size(); ++index) {
      int from = links[index].from;
      char symbol = links[index].symbol;
      int to = links[index].to;
      transitions[from][symbol - 'a'] = to;
    }
  }

  bool IsEquivalent (const Automaton& other) const {
    deque <int> bad_state = Distinguishable(other);
    vector <bool> equal_states(states_count * other.states_count, true);
    for (int index = 0; index < bad_state.size(); ++index) {
      equal_states[bad_state[index]] = false;
    }
    vector <vector<int> > graph = Create_graph(other);
    while (!bad_state.empty()) {
      int vertex = bad_state.front();
      if (vertex == start_index) {
        return false;
      }
      bad_state.pop_front();
      for (int index = 0; index < graph[vertex].size(); ++index) {
        int to = graph[vertex][index];
        if (equal_states[to]) {
          equal_states[to] = false;
          bad_state.push_back(to);
        }
      }
    }
    return true;
  }
};

Automaton ReadAutomaton() {
  int size, exit_number, alphabet;
  cin >> size >> exit_number >> alphabet;
  vector <int> exits(exit_number);
  for (int index = 0; index < exit_number; ++index) {
    cin >> exits[index];
  }
  vector <Transition> links(size * alphabet);
  for (int index = 0; index < size * alphabet; ++index) {
    cin >> links[index].from >> links[index].symbol >> links[index].to;
  }
  return Automaton(size, alphabet, 0, exits, links);
}

int main() {
  std::ios::sync_with_stdio (false);
  Automaton first = ReadAutomaton();
  Automaton second = ReadAutomaton();
  bool are_equivalent = first.IsEquivalent(second);
  std::cout << (are_equivalent ? "EQUIVALENT" : "NOT EQUIVALENT") << std::endl;
  return 0;
}
