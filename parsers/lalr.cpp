#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

struct Production
{
    string lhs;
    vector<string> rhs;
};

struct LR1Item
{
    string lhs;
    vector<string> rhs;
    int dot;
    string lookahead;

    bool operator<(const LR1Item &other) const
    {
        return tie(lhs, rhs, dot, lookahead) < tie(other.lhs, other.rhs, other.dot, other.lookahead);
    }

    bool operator==(const LR1Item &other) const
    {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot && lookahead == other.lookahead;
    }
};

struct Core
{
    string lhs;
    vector<string> rhs;
    int dot;

    bool operator<(const Core &other) const
    {
        return tie(lhs, rhs, dot) < tie(other.lhs, other.rhs, other.dot);
    }

    bool operator==(const Core &other) const
    {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot;
    }
};

vector<Production> grammar;
set<string> terminals, nonTerminals;
map<pair<int, string>, string> actionTable;
map<pair<int, string>, int> gotoTable;
map<string, set<string>> firstCache;

vector<string> tokenize(const string &s)
{
    stringstream ss(s);
    string token;
    vector<string> result;
    while (ss >> token)
        result.push_back(token);
    return result;
}

set<string> computeFirst(const vector<string> &symbols)
{
    set<string> first;
    if (symbols.empty())
    {
        first.insert("");
        return first;
    }

    string key = "";
    for (const auto &sym : symbols)
        key += sym + " ";
    if (firstCache.count(key))
        return firstCache[key];

    string firstSymbol = symbols[0];
    if (terminals.count(firstSymbol))
    {
        first.insert(firstSymbol);
    }
    else if (nonTerminals.count(firstSymbol))
    {
        for (const Production &prod : grammar)
        {
            if (prod.lhs == firstSymbol)
            {
                vector<string> rest(prod.rhs.begin(), prod.rhs.end());
                auto subFirst = computeFirst(rest);
                first.insert(subFirst.begin(), subFirst.end());
            }
        }
    }

    if (first.count("") && symbols.size() > 1)
    {
        vector<string> rest(symbols.begin() + 1, symbols.end());
        auto restFirst = computeFirst(rest);
        first.erase("");
        first.insert(restFirst.begin(), restFirst.end());
    }

    firstCache[key] = first;
    return first;
}

set<LR1Item> closure(set<LR1Item> items)
{
    bool added;
    do
    {
        added = false;
        set<LR1Item> newItems = items;
        for (const LR1Item &item : items)
        {
            if (item.dot < item.rhs.size())
            {
                string B = item.rhs[item.dot];
                if (nonTerminals.count(B))
                {
                    vector<string> beta(item.rhs.begin() + item.dot + 1, item.rhs.end());
                    set<string> firstBeta;
                    if (beta.empty())
                    {
                        firstBeta.insert(item.lookahead);
                    }
                    else
                    {
                        firstBeta = computeFirst(beta);
                        if (firstBeta.count(""))
                        {
                            firstBeta.erase("");
                            firstBeta.insert(item.lookahead);
                        }
                    }
                    for (const Production &prod : grammar)
                    {
                        if (prod.lhs == B)
                        {
                            for (const string &t : firstBeta)
                            {
                                if (!t.empty())
                                {
                                    LR1Item newItem = {prod.lhs, prod.rhs, 0, t};
                                    if (!newItems.count(newItem))
                                    {
                                        newItems.insert(newItem);
                                        added = true;
                                    }
                                }
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

set<LR1Item> goTo(const set<LR1Item> &items, const string &symbol)
{
    set<LR1Item> nextItems;
    for (const LR1Item &item : items)
    {
        if (item.dot < item.rhs.size() && item.rhs[item.dot] == symbol)
        {
            LR1Item moved = item;
            moved.dot++;
            nextItems.insert(moved);
        }
    }
    return closure(nextItems);
}

void printItemSets(const vector<set<LR1Item>> &states)
{
    cout << "\nItem Sets (States):\n";
    for (int i = 0; i < states.size(); ++i)
    {
        cout << "\nState " << i << ":\n";
        for (const LR1Item &item : states[i])
        {
            cout << item.lhs << " -> ";
            for (int j = 0; j < item.rhs.size(); ++j)
            {
                if (j == item.dot)
                    cout << ". ";
                cout << item.rhs[j] << " ";
            }
            if (item.dot == item.rhs.size())
                cout << ".";
            cout << ", " << item.lookahead << endl;
        }
    }
}

void printParseTable()
{
    vector<string> symbols(terminals.begin(), terminals.end());
    for (const string &nt : nonTerminals)
    {
        if (nt != "S'")
            symbols.push_back(nt);
    }

    cout << "\nLALR(1) Parsing Table:\n";
    cout << "State\t";
    for (const string &sym : symbols)
    {
        cout << sym << "\t";
    }
    cout << "\n------------------------------------------------------\n";

    int maxState = 0;
    for (const auto &entry : actionTable)
        maxState = max(maxState, entry.first.first);
    for (const auto &entry : gotoTable)
        maxState = max(maxState, entry.first.first);

    for (int i = 0; i <= maxState; ++i)
    {
        cout << i << "\t";
        for (const string &sym : symbols)
        {
            string entry;
            if (actionTable.count({i, sym}))
            {
                entry = actionTable[{i, sym}];
            }
            else if (gotoTable.count({i, sym}))
            {
                entry = to_string(gotoTable[{i, sym}]);
            }
            cout << entry << "\t";
        }
        cout << "\n";
    }
}

void buildParser()
{
    grammar.insert(grammar.begin(), {"S'", {"S"}}); // Production 0: S' -> S
    nonTerminals.insert("S'");
    terminals.insert("$");

    // Build LR(1) item sets
    set<LR1Item> start = closure({{"S'", {"S"}, 0, "$"}});
    vector<set<LR1Item>> lr1States = {start};
    map<pair<int, string>, int> lr1Transitions;

    for (int i = 0; i < lr1States.size(); ++i)
    {
        set<string> symbols;
        for (const LR1Item &item : lr1States[i])
        {
            if (item.dot < item.rhs.size())
                symbols.insert(item.rhs[item.dot]);
        }
        for (const string &sym : symbols)
        {
            set<LR1Item> next = goTo(lr1States[i], sym);
            if (!next.empty())
            {
                auto it = find(lr1States.begin(), lr1States.end(), next);
                int index;
                if (it == lr1States.end())
                {
                    lr1States.push_back(next);
                    index = lr1States.size() - 1;
                }
                else
                {
                    index = distance(lr1States.begin(), it);
                }
                lr1Transitions[{i, sym}] = index;
            }
        }
    }

    // Merge LR(1) states to LALR(1) states
    map<set<Core>, int> coreToLalrState;
    vector<set<LR1Item>> lalrStates;
    map<int, int> lr1ToLalr;

    for (int i = 0; i < lr1States.size(); ++i)
    {
        set<Core> core;
        for (const LR1Item &item : lr1States[i])
        {
            core.insert({item.lhs, item.rhs, item.dot});
        }
        if (coreToLalrState.count(core))
        {
            int lalrIndex = coreToLalrState[core];
            lalrStates[lalrIndex].insert(lr1States[i].begin(), lr1States[i].end());
            lr1ToLalr[i] = lalrIndex;
        }
        else
        {
            coreToLalrState[core] = lalrStates.size();
            lr1ToLalr[i] = lalrStates.size();
            lalrStates.push_back(lr1States[i]);
        }
    }

    // Update transitions for LALR(1)
    map<pair<int, string>, int> lalrTransitions;
    for (const auto &t : lr1Transitions)
    {
        int from = lr1ToLalr[t.first.first];
        string sym = t.first.second;
        int to = lr1ToLalr[t.second];
        lalrTransitions[{from, sym}] = to;
    }

    bool conflict = false;

    // Build action and goto tables
    for (int i = 0; i < lalrStates.size(); ++i)
    {
        for (const LR1Item &item : lalrStates[i])
        {
            if (item.dot < item.rhs.size())
            {
                string sym = item.rhs[item.dot];
                if (terminals.count(sym) && lalrTransitions.count({i, sym}))
                {
                    string action = "s" + to_string(lalrTransitions[{i, sym}]);
                    if (actionTable.count({i, sym}) && actionTable[{i, sym}] != action)
                    {
                        cout << "Conflict at state " << i << " on symbol '" << sym << "': " << actionTable[{i, sym}] << " vs " << action << endl;
                        conflict = true;
                    }
                    actionTable[{i, sym}] = action;
                }
            }
            else
            {
                if (item.lhs == "S'" && item.lookahead == "$")
                {
                    actionTable[{i, "$"}] = "acc";
                }
                else
                {
                    for (int j = 0; j < grammar.size(); ++j)
                    {
                        if (grammar[j].lhs == item.lhs && grammar[j].rhs == item.rhs)
                        {
                            string action = "r" + to_string(j);
                            if (actionTable.count({i, item.lookahead}) && actionTable[{i, item.lookahead}] != action)
                            {
                                cout << "Conflict at state " << i << " on symbol '" << item.lookahead << "': " << actionTable[{i, item.lookahead}] << " vs " << action << endl;
                                conflict = true;
                            }
                            actionTable[{i, item.lookahead}] = action;
                        }
                    }
                }
            }
        }
        for (const auto &t : lalrTransitions)
        {
            if (t.first.first == i && nonTerminals.count(t.first.second))
            {
                gotoTable[{i, t.first.second}] = t.second;
            }
        }
    }

    printItemSets(lalrStates);
    printParseTable();

    if (conflict)
    {
        cout << "\nGrammar has shift/reduce conflicts.\n";
    }
}

bool parseInput(const string &inputStr)
{
    stack<int> st;
    st.push(0);
    vector<string> input = tokenize(inputStr);
    input.push_back("$");
    int ip = 0;

    while (true)
    {
        int state = st.top();
        string sym = input[ip];
        string action = actionTable[{state, sym}];

        if (action.empty())
        {
            cout << "Rejected!\n";
            return false;
        }
        else if (action[0] == 's')
        {
            int next = stoi(action.substr(1));
            st.push(next);
            ip++;
        }
        else if (action[0] == 'r')
        {
            int prodIndex = stoi(action.substr(1));
            Production p = grammar[prodIndex];
            for (int i = 0; i < p.rhs.size(); ++i)
                st.pop();
            int top = st.top();
            if (!gotoTable.count({top, p.lhs}))
            {
                cout << "Rejected!\n";
                return false;
            }
            st.push(gotoTable[{top, p.lhs}]);
        }
        else if (action == "acc")
        {
            cout << "Accepted!\n";
            return true;
        }
    }
}

int main()
{
    cout << "Enter number of productions: ";
    int n;
    cin >> n;
    cin.ignore();
    cout << "Enter productions (e.g., S -> A B):\n";
    for (int i = 0; i < n; ++i)
    {
        string line;
        getline(cin, line);
        size_t arrow = line.find("->");
        string lhs = line.substr(0, arrow - 1);
        string rhsStr = line.substr(arrow + 2);
        vector<string> rhs = tokenize(rhsStr);
        grammar.push_back({lhs, rhs});
        nonTerminals.insert(lhs);
        for (const string &sym : rhs)
        {
            if (sym.empty())
                continue;
            if (!isupper(sym[0]))
                terminals.insert(sym);
            else
                nonTerminals.insert(sym);
        }
    }

    buildParser();

    string input;
    cout << "\nEnter input string (tokens separated by space): ";
    getline(cin, input);
    parseInput(input);
    return 0;
}