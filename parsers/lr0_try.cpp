#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

struct Production {
    string lhs;
    vector<string> rhs;
};

struct Item {
    string lhs;
    vector<string> rhs;
    int dot;

    bool operator<(const Item& other) const {
        return tie(lhs, rhs, dot) < tie(other.lhs, other.rhs, other.dot);
    }

    bool operator==(const Item& other) const {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot;
    }
};

vector<Production> grammar;
set<string> terminals, nonTerminals;
map<pair<int, string>, string> actionTable;
map<pair<int, string>, int> gotoTable;

vector<string> tokenize(const string& s) {
    stringstream ss(s);
    string token;
    vector<string> result;
    while (ss >> token) result.push_back(token);
    return result;
}

set<Item> closure(set<Item> items) {
    bool added;
    do {
        added = false;
        set<Item> newItems = items;
        for (const Item& item : items) {
            if (item.dot < item.rhs.size()) {
                string B = item.rhs[item.dot];
                if (nonTerminals.count(B)) {
                    for (const Production& prod : grammar) {
                        if (prod.lhs == B) {
                            Item newItem = {prod.lhs, prod.rhs, 0};
                            if (!newItems.count(newItem)) {
                                newItems.insert(newItem);
                                added = true;
                            }
                        }
                    }
                }
            }
        }
        items = newItems;
    } while (added);
    return items;
}

set<Item> goTo(const set<Item>& items, const string& symbol) {
    set<Item> nextItems;
    for (const Item& item : items) {
        if (item.dot < item.rhs.size() && item.rhs[item.dot] == symbol) {
            Item moved = item;
            moved.dot++;
            nextItems.insert(moved);
        }
    }
    return closure(nextItems);
}

int getStateIndex(const set<Item>& state, const vector<set<Item>>& states) {
    for (int i = 0; i < states.size(); ++i) {
        if (states[i] == state) return i;
    }
    return -1;
}

void printItemSets(const vector<set<Item>>& states) {
    cout << "\nItem Sets (States):\n";
    for (int i = 0; i < states.size(); ++i) {
        cout << "\nState " << i << ":\n";
        for (const Item& item : states[i]) {
            cout << item.lhs << " -> ";
            for (int j = 0; j < item.rhs.size(); ++j) {
                if (j == item.dot) cout << ". ";
                cout << item.rhs[j] << " ";
            }
            if (item.dot == item.rhs.size()) cout << ".";
            cout << endl;
        }
    }
}

void printParseTable(const vector<set<Item>>& states) {
    vector<string> symbols(terminals.begin(), terminals.end());
    for (const string& nt : nonTerminals) {
        if (nt != "S'") symbols.push_back(nt);
    }

    cout << "\nLR(0) Parsing Table:\n";
    cout << "State\t";
    for (const string& sym : symbols) {
        cout << sym << "\t";
    }
    cout << "\n------------------------------------------------------\n";

    for (int i = 0; i < states.size(); ++i) {
        cout << i << "\t";
        for (const string& sym : symbols) {
            string entry;
            if (actionTable.count({i, sym})) {
                entry = actionTable[{i, sym}];
            } else if (gotoTable.count({i, sym})) {
                entry = to_string(gotoTable[{i, sym}]);
            } else {
                entry = "";
            }
            cout << entry << "\t";
        }
        cout << "\n";
    }
}

void buildParser() {
    grammar.insert(grammar.begin(), {"S'", {"S"}}); // augmented grammar
    nonTerminals.insert("S'");
    set<Item> start = closure({{"S'", {"S"}, 0}});
    vector<set<Item>> states = {start};
    map<pair<int, string>, int> transitions;

    for (int i = 0; i < states.size(); ++i) {
        set<string> symbols;
        for (const Item& item : states[i]) {
            if (item.dot < item.rhs.size()) symbols.insert(item.rhs[item.dot]);
        }
        for (const string& sym : symbols) {
            set<Item> next = goTo(states[i], sym);
            if (!next.empty()) {
                int index = getStateIndex(next, states);
                if (index == -1) {
                    states.push_back(next);
                    index = states.size() - 1;
                }
                transitions[{i, sym}] = index;
            }
        }
    }

    bool conflict = false;

    for (int i = 0; i < states.size(); ++i) {
        for (const Item& item : states[i]) {
            if (item.dot < item.rhs.size()) {
                string sym = item.rhs[item.dot];
                if (terminals.count(sym)) {
                    string action = "s" + to_string(transitions[{i, sym}]);
                    if (actionTable.count({i, sym}) && actionTable[{i, sym}] != action) {
                        cout << "Conflict at state " << i << " on symbol '" << sym << "': " << actionTable[{i, sym}] << " vs " << action << endl;
                        conflict = true;
                    }
                    actionTable[{i, sym}] = action;
                }
            } else {
                if (item.lhs == "S'") {
                    actionTable[{i, "$"}] = "acc";
                } else {
                    for (const string& t : terminals) {
                        for (int j = 0; j < grammar.size(); ++j) {
                            if (grammar[j].lhs == item.lhs && grammar[j].rhs == item.rhs) {
                                string action = "r" + to_string(j);
                                if (actionTable.count({i, t}) && actionTable[{i, t}] != action) {
                                    cout << "Conflict at state " << i << " on symbol '" << t << "': " << actionTable[{i, t}] << " vs " << action << endl;
                                    conflict = true;
                                }
                                actionTable[{i, t}] = action;
                            }
                        }
                    }
                }
            }
        }
    }

    for (const auto& t : transitions) {
        if (nonTerminals.count(t.first.second)) {
            gotoTable[t.first] = t.second;
        }
    }

    printItemSets(states);
    printParseTable(states);

    if (conflict) {
        cout << "\nGrammar has shift/reduce conflicts.\n";
    }
}

bool parseInput(const string& inputStr) {
    stack<int> st;
    st.push(0);
    vector<string> input = tokenize(inputStr);
    input.push_back("$");
    int ip = 0;

    while (true) {
        int state = st.top();
        string sym = input[ip];
        string action = actionTable[{state, sym}];

        if (action.empty()) {
            cout << "Rejected!\n";
            return false;
        } else if (action[0] == 's') {
            int next = stoi(action.substr(1));
            st.push(next);
            ip++;
        } else if (action[0] == 'r') {
            int prodIndex = stoi(action.substr(1));
            Production p = grammar[prodIndex];
            for (int i = 0; i < p.rhs.size(); ++i) st.pop();
            int top = st.top();
            st.push(gotoTable[{top, p.lhs}]);
        } else if (action == "acc") {
            cout << "Accepted!\n";
            return true;
        }
    }
}

int main() {
    cout << "Enter number of productions: ";
    int n; cin >> n;
    cin.ignore();
    cout << "Enter productions (e.g., S -> A B):\n";
    for (int i = 0; i < n; ++i) {
        string line; getline(cin, line);
        size_t arrow = line.find("->");
        string lhs = line.substr(0, arrow - 1);
        string rhsStr = line.substr(arrow + 2);
        vector<string> rhs = tokenize(rhsStr);
        grammar.push_back({lhs, rhs});
        nonTerminals.insert(lhs);
        for (const string& sym : rhs) {
            if (sym.empty()) continue;
            if (!isupper(sym[0])) terminals.insert(sym);
            else nonTerminals.insert(sym);
        }
    }

    terminals.insert("$");

    buildParser();

    string input;
    cout << "\nEnter input string (tokens separated by space): ";
    getline(cin, input);
    parseInput(input);
    return 0;
}
