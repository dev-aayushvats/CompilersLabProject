#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

// Represents a production rule in the grammar (e.g., S -> A B)
struct Production
{
    string lhs;         // Left-hand side (non-terminal)
    vector<string> rhs; // Right-hand side (sequence of symbols)
};

// Represents an LR(1) item: [A -> α . β, a], where 'a' is the lookahead
struct Item
{
    string lhs;         // Left-hand side of the production
    vector<string> rhs; // Right-hand side of the production
    int dot;            // Position of the dot in the production
    string lookahead;   // Lookahead terminal for CLR(1)

    // Comparison operator for storing items in a set
    bool operator<(const Item &other) const
    {
        return tie(lhs, rhs, dot, lookahead) < tie(other.lhs, other.rhs, other.dot, other.lookahead);
    }

    // Equality operator for checking item equivalence
    bool operator==(const Item &other) const
    {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot && lookahead == other.lookahead;
    }
};

// Global variables
vector<Production> grammar;                 // List of productions
set<string> terminals, nonTerminals;        // Sets of terminals and non-terminals
map<string, set<string>> first;             // First sets for symbols
map<pair<int, string>, string> actionTable; // Action table: (state, terminal) -> action
map<pair<int, string>, int> gotoTable;      // Goto table: (state, non-terminal) -> state

// Tokenizes a string into a vector of symbols
vector<string> tokenize(const string &s)
{
    stringstream ss(s);
    string token;
    vector<string> result;
    while (ss >> token)
        result.push_back(token);
    return result;
}

// Computes first sets for all non-terminals
void computeFirst()
{
    for (const string &t : terminals)
        first[t].insert(t);
    bool changed;
    do
    {
        changed = false;
        for (const Production &prod : grammar)
        {
            string A = prod.lhs;
            const vector<string> &beta = prod.rhs;
            set<string> prev = first[A];
            if (beta.empty())
                first[A].insert(""); // Empty production adds epsilon
            else
            {
                for (int i = 0; i < beta.size(); ++i)
                {
                    const set<string> &f = first[beta[i]];
                    for (const string &s : f)
                    {
                        if (s != "")
                            first[A].insert(s);
                    }
                    if (!f.count(""))
                        break; // Stop if symbol can't derive epsilon
                    if (i == beta.size() - 1)
                        first[A].insert("");
                }
            }
            if (prev != first[A])
                changed = true;
        }
    } while (changed);
}

// Computes the first set of a sequence followed by a lookahead
set<string> getFirstOfSequence(const vector<string> &sequence, const string &lookahead)
{
    set<string> result;
    bool all_nullable = true;
    for (const string &sym : sequence)
    {
        if (terminals.count(sym))
        {
            result.insert(sym);
            all_nullable = false;
            break;
        }
        else if (nonTerminals.count(sym))
        {
            const set<string> &f = first[sym];
            for (const string &t : f)
            {
                if (t != "")
                    result.insert(t);
            }
            if (!f.count(""))
            {
                all_nullable = false;
                break;
            }
        }
        else
        {
            // Treat unknown symbols as terminals
            result.insert(sym);
            all_nullable = false;
            break;
        }
    }
    if (all_nullable)
        result.insert(lookahead); // Include lookahead if sequence can derive epsilon
    return result;
}

// Computes the closure of a set of items
set<Item> closure(set<Item> items)
{
    bool added;
    do
    {
        added = false;
        set<Item> newItems = items;
        for (const Item &item : items)
        {
            if (item.dot < item.rhs.size())
            {
                string B = item.rhs[item.dot];
                if (nonTerminals.count(B))
                {
                    vector<string> beta(item.rhs.begin() + item.dot + 1, item.rhs.end());
                    set<string> lookaheads = getFirstOfSequence(beta, item.lookahead);
                    for (const Production &prod : grammar)
                    {
                        if (prod.lhs == B)
                        {
                            for (const string &b : lookaheads)
                            {
                                Item newItem = {prod.lhs, prod.rhs, 0, b};
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
        items = newItems;
    } while (added);
    return items;
}

// Computes the goto set for a set of items on a symbol
set<Item> goTo(const set<Item> &items, const string &symbol)
{
    set<Item> nextItems;
    for (const Item &item : items)
    {
        if (item.dot < item.rhs.size() && item.rhs[item.dot] == symbol)
        {
            Item moved = item;
            moved.dot++;
            nextItems.insert(moved);
        }
    }
    return closure(nextItems);
}

// Finds the index of a state in the states vector
int getStateIndex(const set<Item> &state, const vector<set<Item>> &states)
{
    for (int i = 0; i < states.size(); ++i)
    {
        if (states[i] == state)
            return i;
    }
    return -1;
}

// Prints all item sets (states)
void printItemSets(const vector<set<Item>> &states)
{
    cout << "\nItem Sets (States):\n";
    for (int i = 0; i < states.size(); ++i)
    {
        cout << "\nState " << i << ":\n";
        for (const Item &item : states[i])
        {
            cout << "[" << item.lhs << " -> ";
            for (int j = 0; j < item.rhs.size(); ++j)
            {
                if (j == item.dot)
                    cout << ". ";
                cout << item.rhs[j] << " ";
            }
            if (item.dot == item.rhs.size())
                cout << ".";
            cout << ", " << item.lookahead << "]" << endl;
        }
    }
}

// Prints the CLR(1) parsing table
void printParseTable(const vector<set<Item>> &states)
{
    vector<string> symbols(terminals.begin(), terminals.end());
    for (const string &nt : nonTerminals)
    {
        if (nt != "S'")
            symbols.push_back(nt);
    }

    cout << "\nCLR(1) Parsing Table:\n";
    cout << "State\t";
    for (const string &sym : symbols)
        cout << sym << "\t";
    cout << "\n------------------------------------------------------\n";

    for (int i = 0; i < states.size(); ++i)
    {
        cout << i << "\t";
        for (const string &sym : symbols)
        {
            string entry;
            if (actionTable.count({i, sym}))
                entry = actionTable[{i, sym}];
            else if (gotoTable.count({i, sym}))
                entry = to_string(gotoTable[{i, sym}]);
            else
                entry = "";
            cout << entry << "\t";
        }
        cout << "\n";
    }
}

// Builds the CLR(1) parser
void buildParser()
{
    string startSymbol = grammar[0].lhs;
    grammar.insert(grammar.begin(), {"S'", {startSymbol}}); // Add augmented production
    nonTerminals.insert("S'");
    computeFirst();
    set<Item> start = closure({{"S'", {startSymbol}, 0, "$"}}); // Start item with lookahead $
    vector<set<Item>> states = {start};
    map<pair<int, string>, int> transitions;

    // Generate all states
    for (int i = 0; i < states.size(); ++i)
    {
        set<string> symbols;
        for (const Item &item : states[i])
        {
            if (item.dot < item.rhs.size())
                symbols.insert(item.rhs[item.dot]);
        }
        for (const string &sym : symbols)
        {
            set<Item> next = goTo(states[i], sym);
            if (!next.empty())
            {
                int index = getStateIndex(next, states);
                if (index == -1)
                {
                    states.push_back(next);
                    index = states.size() - 1;
                }
                transitions[{i, sym}] = index;
            }
        }
    }

    bool conflict = false;

    // Populate action and goto tables
    for (int i = 0; i < states.size(); ++i)
    {
        for (const Item &item : states[i])
        {
            if (item.dot < item.rhs.size())
            {
                string sym = item.rhs[item.dot];
                if (terminals.count(sym))
                {
                    string action = "s" + to_string(transitions[{i, sym}]);
                    if (actionTable.count({i, sym}))
                    {
                        if (actionTable[{i, sym}] != action)
                        {
                            cout << "Conflict at state " << i << " on symbol '" << sym << "': "
                                 << actionTable[{i, sym}] << " vs " << action << endl;
                            conflict = true;
                        }
                    }
                    else
                    {
                        actionTable[{i, sym}] = action;
                    }
                }
            }
            else
            {
                if (item.lhs == "S'" && item.lookahead == "$")
                {
                    if (actionTable.count({i, "$"}))
                    {
                        cout << "Conflict at state " << i << " on symbol '$': "
                             << actionTable[{i, "$"}] << " vs acc" << endl;
                        conflict = true;
                    }
                    else
                    {
                        actionTable[{i, "$"}] = "acc";
                    }
                }
                else
                {
                    for (int j = 0; j < grammar.size(); ++j)
                    {
                        if (grammar[j].lhs == item.lhs && grammar[j].rhs == item.rhs)
                        {
                            string t = item.lookahead;
                            string action = "r" + to_string(j);
                            if (actionTable.count({i, t}))
                            {
                                if (actionTable[{i, t}] != action)
                                {
                                    cout << "Conflict at state " << i << " on symbol '" << t << "': "
                                         << actionTable[{i, t}] << " vs " << action << endl;
                                    conflict = true;
                                }
                            }
                            else
                            {
                                actionTable[{i, t}] = action;
                            }
                        }
                    }
                }
            }
        }
    }

    // Populate goto table for non-terminals
    for (const auto &t : transitions)
    {
        if (nonTerminals.count(t.first.second))
        {
            gotoTable[t.first] = t.second;
        }
    }

    printItemSets(states);
    printParseTable(states);

    if (conflict)
        cout << "\nGrammar has conflicts.\n";
    else
        cout << "\nGrammar is CLR(1) with no conflicts.\n";
}

// Parses an input string using the CLR(1) parsing table
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
            st.push(gotoTable[{top, p.lhs}]);
        }
        else if (action == "acc")
        {
            cout << "Accepted!\n";
            return true;
        }
    }
}

// Main function to input grammar and test parsing
int main()
{
    cout << "Enter number of productions: ";
    int n;
    cin >> n;
    cin.ignore();
    cout << "Note: Please enter productions with spaces between symbols (e.g., S -> A B C).\n";
    cout << "Enter productions (e.g., S -> A B):\n";
    for (int i = 0; i < n; ++i)
    {
        string line;
        getline(cin, line);
        size_t arrow = line.find("->");
        string lhs = line.substr(0, arrow);
        string rhsStr = line.substr(arrow + 2);
        while (!lhs.empty() && isspace(lhs.back()))
            lhs.pop_back();
        while (!lhs.empty() && isspace(lhs.front()))
            lhs.erase(0, 1);
        while (!rhsStr.empty() && isspace(rhsStr.back()))
            rhsStr.pop_back();
        while (!rhsStr.empty() && isspace(rhsStr.front()))
            rhsStr.erase(0, 1);
        vector<string> rhs = tokenize(rhsStr);
        grammar.push_back({lhs, rhs});
        nonTerminals.insert(lhs);
        for (const string &sym : rhs)
        {
            if (sym.empty())
                continue;
            if (!isupper(sym[0]))
                terminals.insert(sym); // Assume lowercase first char is terminal
            else
                nonTerminals.insert(sym);
        }
    }

    terminals.insert("$"); // Add end marker

    buildParser();

    string input;
    cout << "\nEnter input string (tokens separated by space): ";
    getline(cin, input);
    parseInput(input);
    return 0;
}