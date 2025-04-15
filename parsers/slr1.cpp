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

struct Item
{
    string lhs;
    vector<string> rhs;
    int dot;

    bool operator<(const Item &other) const
    {
        return tie(lhs, rhs, dot) < tie(other.lhs, other.rhs, other.dot);
    }

    bool operator==(const Item &other) const
    {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot;
    }
};

vector<Production> grammar;
set<string> terminals, nonTerminals;
map<string, set<string>> first, follow;
map<pair<int, string>, string> actionTable;
map<pair<int, string>, int> gotoTable;

vector<string> tokenize(const string &s)
{
    stringstream ss(s);
    string token;
    vector<string> result;
    while (ss >> token)
        result.push_back(token);
    return result;
}

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
                first[A].insert("");
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
                        break;
                    if (i == beta.size() - 1)
                        first[A].insert("");
                }
            }
            if (prev != first[A])
                changed = true;
        }
    } while (changed);
}

void computeFollow()
{
    follow["S'"].insert("$");
    bool changed;
    do
    {
        changed = false;
        for (const Production &prod : grammar)
        {
            string A = prod.lhs;
            const vector<string> &beta = prod.rhs;
            for (int i = 0; i < beta.size(); ++i)
            {
                if (nonTerminals.count(beta[i]))
                {
                    set<string> prev = follow[beta[i]];
                    if (i + 1 < beta.size())
                    {
                        const set<string> &f = first[beta[i + 1]];
                        for (const string &s : f)
                        {
                            if (s != "")
                                follow[beta[i]].insert(s);
                        }
                        if (f.count(""))
                        {
                            for (const string &s : follow[A])
                            {
                                follow[beta[i]].insert(s);
                            }
                        }
                    }
                    else
                    {
                        for (const string &s : follow[A])
                        {
                            follow[beta[i]].insert(s);
                        }
                    }
                    if (prev != follow[beta[i]])
                        changed = true;
                }
            }
        }
    } while (changed);
}

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
                    for (const Production &prod : grammar)
                    {
                        if (prod.lhs == B)
                        {
                            Item newItem = {prod.lhs, prod.rhs, 0};
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
        items = newItems;
    } while (added);
    return items;
}

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

int getStateIndex(const set<Item> &state, const vector<set<Item>> &states)
{
    for (int i = 0; i < states.size(); ++i)
    {
        if (states[i] == state)
            return i;
    }
    return -1;
}

void printItemSets(const vector<set<Item>> &states)
{
    cout << "\nItem Sets (States):\n";
    for (int i = 0; i < states.size(); ++i)
    {
        cout << "\nState " << i << ":\n";
        for (const Item &item : states[i])
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
            cout << endl;
        }
    }
}

void printParseTable(const vector<set<Item>> &states)
{
    vector<string> symbols(terminals.begin(), terminals.end());
    for (const string &nt : nonTerminals)
    {
        if (nt != "S'")
            symbols.push_back(nt);
    }

    cout << "\nSLR(1) Parsing Table:\n";
    cout << "State\t";
    for (const string &sym : symbols)
    {
        cout << sym << "\t";
    }
    cout << "\n------------------------------------------------------\n";

    for (int i = 0; i < states.size(); ++i)
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
            else
            {
                entry = "";
            }
            cout << entry << "\t";
        }
        cout << "\n";
    }
}

void buildParser()
{
    string startSymbol = grammar[0].lhs;
    grammar.insert(grammar.begin(), {"S'", {startSymbol}}); // augmented grammar
    nonTerminals.insert("S'");
    computeFirst();
    computeFollow();
    set<Item> start = closure({{"S'", {startSymbol}, 0}});
    vector<set<Item>> states = {start};
    map<pair<int, string>, int> transitions;

    // Build states and transitions
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

    // Populate action and goto tables with corrected conflict detection
    for (int i = 0; i < states.size(); ++i)
    {
        for (const Item &item : states[i])
        {
            if (item.dot < item.rhs.size())
            {
                // Shift action
                string sym = item.rhs[item.dot];
                if (terminals.count(sym))
                {
                    string action = "s" + to_string(transitions[{i, sym}]);
                    if (actionTable.count({i, sym}))
                    {
                        // Conflict if existing action is a reduce
                        if (actionTable[{i, sym}][0] == 'r')
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
                // Reduce or accept action
                if (item.lhs == "S'")
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
                            for (const string &t : follow[item.lhs])
                            {
                                string action = "r" + to_string(j);
                                if (actionTable.count({i, t}))
                                {
                                    // Conflict if existing action is shift or another reduce
                                    if (actionTable[{i, t}][0] == 's' || actionTable[{i, t}][0] == 'r')
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
    }

    // Populate goto table
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
    {
        cout << "\nGrammar has shift/reduce conflicts.\n";
    }
    else
    {
        cout << "\nGrammar is SLR(1) with no conflicts.\n";
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
    cout << "Note: Please enter productions with spaces between symbols (e.g., S -> A B C)." << endl;
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
                terminals.insert(sym);
            else
                nonTerminals.insert(sym);
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