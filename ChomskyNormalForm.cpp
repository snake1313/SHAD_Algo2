#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <assert.h>
#include <memory>

using std::vector;
using std::string;
using std::cin;
using std::unique_ptr;

class ContextFreeGrammar {

  static const int ALPHABET_SIZE = 'z' - 'a' + 1;
  static const char INITIAL_START_SYMBOL = 'S';
  int current_nonterminals_count;
  
  struct Symbol {
    int id;
    bool is_terminal;
    Symbol() {
      id = -1;
    }
    explicit Symbol(char letter) {
      if (letter >= 'a' && letter <= 'z') {
        id = letter - 'a';
        is_terminal = true;
      }
      else {
        id = letter - 'A';
        is_terminal = false;
      }
    }
    Symbol(int rank, bool is_it_terminal) {
      id = rank;
      is_terminal = is_it_terminal;
    }
    char ToChar() const {
      return (is_terminal ? 'a' + id : 'A' + id);
    }
  };

  class Rule {
    Symbol left;
    vector<Symbol> right;
  public:
    Rule() {
      left = Symbol();
    }
    Rule(Symbol left_part, vector<Symbol> right_part) {
      left = left_part;
      right = right_part;
    }
    int GetLeftId() const {
      return left.id;
    }
    Symbol GetRightSymbol (int position) const {
      return right[position];
    }
    vector<Symbol> GetRightPart() const {
      return right;
    }
    int Size() const {
      return right.size();
    }
  };

  vector <Rule> data;

  static Rule OneSymbolRule(Symbol left, Symbol right) {
    return Rule(left, vector<Symbol>(1, right));
  }

  void DeleteLongRules() {
    vector<Rule> new_data;
    for (auto const& rule : data) {
      int length = rule.Size();
      if (length <= 2) {
        new_data.push_back(rule);
        continue;
      }
      for (int position = 0; position < length - 1; ++position) {
        Symbol left, first, second;
        first = rule.GetRightSymbol(position);
        if (position != 0) {
          left = Symbol(current_nonterminals_count + position - 1, false);
        }
        else {
          left = Symbol(rule.GetLeftId(), false);
        }
        if (position < length - 2) {
          second = Symbol(current_nonterminals_count + position, false);
        }
        else {
          second = rule.GetRightSymbol(length - 1);
        }
        vector<Symbol> right;
        right.push_back(first);
        right.push_back(second);
        new_data.push_back(Rule(left, right));
      }
      current_nonterminals_count += (length - 2);
    }
    data = std::move(new_data);
  }

  std::set<int> nullables;

  bool IsItNullable (Symbol symbol) const {
    return !symbol.is_terminal && nullables.count(symbol.id) != 0;
  }

  bool IsItEPSRule(const Rule& rule) const {
    return rule.Size() == 0;
  }

  void FindNullable() {
    bool stop = false;
    while (!stop) {
      stop = true;
      for (auto const& rule : data) {
        int position = 0;
        for (; position < rule.Size(); ++position) {
          if (!IsItNullable(rule.GetRightSymbol(position))) {
            break;
          }
        }
        if (position == rule.Size()) {
          if (nullables.find(rule.GetLeftId()) == nullables.end()) {
            nullables.insert(rule.GetLeftId());
            stop = false;
          }
        }
      }
    }
  }

  void DeleteEPSRules() {
    FindNullable();
    vector<Rule> new_data;
    for (auto const& rule : data) {
      if (IsItEPSRule(rule)) {
        continue;
      }
      new_data.push_back(rule);
      if (rule.Size() == 2) {
        Symbol first = rule.GetRightSymbol(0);
        Symbol second = rule.GetRightSymbol(1);
        int id = rule.GetLeftId();
        if (IsItNullable(first)) {
          new_data.push_back(OneSymbolRule(Symbol(id, false), second));
        }
        if (IsItNullable(second)) {
          new_data.push_back(OneSymbolRule(Symbol(id, false), first));
        }
      }
    }
    data = std::move(new_data);
  }

  bool IsItChainRule(const Rule& rule) const {
    Symbol first = rule.GetRightSymbol(0);
    return rule.Size() == 1 && !first.is_terminal;
  }

  vector <vector<int> > FindChainRules() const {
    vector <vector<int> > graph(current_nonterminals_count, vector<int>());
    vector<vector<int> > ancestors = graph;
    for (auto const& rule : data) {
      if (IsItChainRule(rule)) {
        graph[rule.GetLeftId()].push_back(rule.GetRightSymbol(0).id);
      }
    }
    for (int vertex = 1; vertex < current_nonterminals_count; ++vertex) {
      vector<bool> used(current_nonterminals_count, false);
      std::queue<int> bfs;
      bfs.push(vertex);
      used[vertex] = true;
      while (!bfs.empty()) {
        int root = bfs.front();
        bfs.pop();
        for (size_t index = 0; index < graph[root].size(); ++index) {
          int to = graph[root][index];
          if (!used[to]) {
            used[to] = true;
            bfs.push(to);
            ancestors[to].push_back(vertex);
          }
        }
      }
    }
    return ancestors;
  }

  void DeleteChainRules() {
    vector<vector<int> > ancestors = FindChainRules();
    vector<Rule> new_data;
    for (auto const& rule : data) {
      if (IsItChainRule(rule)) {
        continue;
      }
      new_data.push_back(rule);
      auto const& current_ancestors = ancestors[rule.GetLeftId()];
      size_t size = current_ancestors.size();
      for (size_t position = 0; position < size; ++position) {
        int from = current_ancestors[position];
        new_data.push_back(Rule(Symbol(from, false), rule.GetRightPart()));
      }
    }
    data = std::move(new_data);
  }

  bool IsItSemiTerminal(const Rule& rule) const {
    if (rule.Size() == 1) {
      return false;
    }
    for (int position = 0; position < rule.Size(); ++position) {
      if (rule.GetRightSymbol(position).is_terminal) {
        return true;
      }
    }
    return false;
  }

  void DeleteSemiTerminals() {
    vector<Rule> new_data;
    for (auto const& rule : data) {
      vector<Symbol> new_right;
      if (IsItSemiTerminal(rule)) {
        for (int position = 0; position < rule.Size(); ++position) {
          Symbol symbol = rule.GetRightSymbol(position);
          if (symbol.is_terminal) {
            Symbol add(current_nonterminals_count, false);
            ++current_nonterminals_count;
            new_data.push_back(OneSymbolRule(add, symbol));
            new_right.push_back(add);
          }
          else {
            new_right.push_back(symbol);
          }
        }
      }
      else {
        new_right = rule.GetRightPart();
      }
      Rule new_rule = Rule(Symbol(rule.GetLeftId(), false), new_right);
      new_data.push_back(new_rule);
    }
    data = new_data;
  }

public:

  explicit ContextFreeGrammar(vector<string> input_data) {
    for (auto const& rule : input_data) {
      Symbol left(rule[0]);
      vector<Symbol> right;
      for (size_t position = 3; position < rule.size(); ++position) {
        right.push_back(Symbol(rule[position]));
      }
      data.push_back(Rule(left, right));
    }
    current_nonterminals_count = ALPHABET_SIZE;
  }

  void ConvertToChomskyNormalForm() {
    DeleteLongRules();
    DeleteEPSRules();
    DeleteChainRules();
    DeleteSemiTerminals();
  }

  vector<Rule> GrammarWithoutOneSymbolRules() const {
    vector<Rule> answer;
    for (auto const& rule : data) {
      if (rule.Size() > 1) {
        answer.push_back(rule);
      }
    }
    return answer;
  }

  unique_ptr<string> LexicographicallyMinimalWord (int length) const {
    if (IsItNullable(Symbol(INITIAL_START_SYMBOL))) {
      return unique_ptr<string>(new string(""));
    }
    const string INF = "|";
    vector<vector<string> > dp(length + 1, vector<string>(current_nonterminals_count, INF));
    for (auto const& rule : data) {
      if (rule.Size() == 1) {
        assert(rule.GetRightSymbol(0).is_terminal);
        int value = rule.GetRightSymbol(0).id;
        string letter = " ";
        letter[0] = rule.GetRightSymbol(0).ToChar();
        int id = rule.GetLeftId();
        dp[1][id] = std::min(dp[1][id], letter);
      }
    }
    vector<Rule> new_data = GrammarWithoutOneSymbolRules();
    for (int size = 2; size <= length; ++size) {
      for (auto const& rule : new_data) {
        int first = rule.GetRightSymbol(0).id;
        int second = rule.GetRightSymbol(1).id;
        int id = rule.GetLeftId();
        for (int piece = 1; piece + 1 <= size; ++piece) {
          string left = dp[piece][first];
          string right = dp[size - piece][second];
          if (left != INF && right != INF) {
            dp[size][id] = std::min(dp[size][id], left + right);
          }
        }
      }
    }
    string answer = INF;
    for (int index = 1; index <= length; ++index) {
      answer = std::min(answer, dp[index][INITIAL_START_SYMBOL - 'A']);
    }
    return (answer != INF ? unique_ptr<string>(new string(answer)) : NULL);
  }
};

vector<string> ReadData(std::istream& in) {
  int size;
  cin >> size;
  vector<string> input_data;
  for (int index = 0; index < size; ++index) {
    string rule;
    cin >> rule;
    if (rule.back() == '$') {
      rule.pop_back();
    }
    input_data.push_back(rule);
  }
  return input_data;
}

int main() {
  ContextFreeGrammar grammar(ReadData(std::cin));
  grammar.ConvertToChomskyNormalForm();
  int length;
  cin >> length;
  unique_ptr<string> result = grammar.LexicographicallyMinimalWord(length);
  if (result != NULL) {
    std::cout << (*result != "" ? *result : "$");
  }
  else {
    std::cout << "IMPOSSIBLE";
  }
  return 0;
}
