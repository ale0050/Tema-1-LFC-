#include "DeterministicFiniteAutomaton.h"
#include "NondeterministicFiniteAutomaton.h"
#include <iostream>
#include <stack>
#include <windows.h>
#include <fstream>
#include <algorithm>

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

#define COLOR_DEFAULT 7  
#define COLOR_BLUE 9
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_RED 12
#define COLOR_CYAN 11
#define COLOR_BOLD 8
#define COLOR_PINK 13

//nod arbore sintactic
struct Node {
    char value;
    Node* left;
    Node* right;
    Node(char v, Node* l = nullptr, Node* r = nullptr)
        : value(v), left(l), right(r) {
    }
};

bool isOperand(char c) { return isalnum((unsigned char)c); }
bool isUnaryOperator(char c) { return c == '*' || c == '+'; }


//inserare concatenare explicita -> adaug . 
string insertConcatenation(const string& regex) 
{
    string processed = "";
    for (size_t i = 0; i < regex.length(); ++i) {
        char curr = regex[i];
        processed += curr;
        if (i + 1 < regex.length()) 
        {
            char next = regex[i + 1];
            bool left = isOperand(curr) || curr == ')' || isUnaryOperator(curr);
            bool right = isOperand(next) || next == '(';
            if (left && right) 
                processed += '.';
        }
    }
    return processed;
}

void setConsoleColor(int color) 
{
    SetConsoleTextAttribute(hConsole, color);
}

int priority(char op) 
{
    switch (op) {
    case '*': case '+': return 3;
    case '.': return 2;
    case '|': return 1;
    default: return 0;
    }
}

NondeterministicFiniteAutomaton regexToNFA_thompson(const string& postfix)
{
    stack<NondeterministicFiniteAutomaton> nfaStack;
    for (char c : postfix) {
        if (isalnum((unsigned char)c))
            nfaStack.push(NondeterministicFiniteAutomaton::createBasicNFA(c));
        else if (c == '.') {
            NondeterministicFiniteAutomaton NFA2 = nfaStack.top(); nfaStack.pop();
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combineConcatenation(NFA2));
        }
        else if (c == '|') {
            NondeterministicFiniteAutomaton NFA2 = nfaStack.top(); nfaStack.pop();
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combineUnion(NFA2));
        }
        else if (c == '*') {
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combineKleeneStar());
        }
        else if (c == '+') {
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combinePlus());
        }
    }

    if (nfaStack.empty())
        throw runtime_error("Expresie regulata invalida sau goala.");

    return nfaStack.top();
}

//regex in postfix
string toPostfix(const string& regex) 
{
    stack<char> operators;
    string out;
    for (char c : regex) {
        if (isOperand(c)) 
            out += c;
        else if (c == '(') 
            operators.push(c);
        else if (c == ')') 
        {
            while (!operators.empty() && operators.top() != '(') 
            {
                out += operators.top();
                operators.pop();
            }
            if (!operators.empty()) 
                operators.pop();
        }
        else {
            while (!operators.empty() && priority(operators.top()) >= priority(c)) {
                out += operators.top();
                operators.pop();
            }
            operators.push(c);
        }
    }
    while (!operators.empty()) {
        out += operators.top();
        operators.pop();
    }
    return out;
}

DeterministicFiniteAutomaton RegexToDFA(const string& regex) {
    string processed_regex = insertConcatenation(regex);
    string postfix_r = toPostfix(processed_regex);
    NondeterministicFiniteAutomaton NFA = regexToNFA_thompson(postfix_r);
    return NFA.convertToDFA();
}

Node* buildSyntaxTree(const string& postfix) {
    stack<Node*> st;
    for (char c : postfix) {
        if (isalnum(c)) st.push(new Node(c));
        else if (isUnaryOperator(c)) {
            Node* a = st.top(); st.pop();
            st.push(new Node(c, a));
        }
        else { // operatori binari: . |
            Node* b = st.top(); st.pop();
            Node* a = st.top(); st.pop();
            st.push(new Node(c, a, b));
        }
    }
    return st.empty() ? nullptr : st.top();
}

void printSyntaxTree(Node* root, string indent = "", bool last = true) {
    if (!root)
        return;

    cout << indent;
    if (last) {
        cout << "-- ";
        indent += "   ";
    }
    else {
        cout << "|-- ";
        indent += "|  ";
    }
    setConsoleColor(COLOR_PINK | COLOR_BOLD);
    cout << root->value << "\n";
    setConsoleColor(COLOR_DEFAULT | COLOR_BOLD);

    if (root->left || root->right) {
        printSyntaxTree(root->left, indent, root->right == nullptr);
        printSyntaxTree(root->right, indent, true);
    }
}

int main()
{
    string regex_r;
    ifstream inFile("regexInput.txt");
    if (inFile.is_open()) {
        getline(inFile, regex_r);
        inFile.close();
        regex_r.erase(remove(regex_r.begin(), regex_r.end(), '\n'), regex_r.end());
        cout << "Expresia regulata citita: " << regex_r << endl;
    }
    else {
        cout << "Fisierul nu exista, folosim valoare implicita: a+b" << endl;
        regex_r = "a+b";
    }

    cout << "---------------------------------------" << endl;

    string processed_regex = insertConcatenation(regex_r);
    string postfix_r = toPostfix(processed_regex);

    DeterministicFiniteAutomaton AFD = RegexToDFA(regex_r);

    if (!AFD.verifyAutomaton()) {
        cerr << "ATENTIE: Automat invalid!" << endl;
    }

    int choice;
    string word_to_check;
    setConsoleColor(COLOR_CYAN | COLOR_BOLD);
    do {
        cout << "\n--- MENIU AFD ---" << endl;
        cout << "1. Afisare forma poloneza postfixata " << endl;
        cout << "2. Afisare arbore sintactic" << endl;
        cout << "3. Afisare automat in consola si fisier" << endl;
        cout << "4. Verificare cuvant in automat " << endl;
        cout << "0. Iesire" << endl;
        cout << "Alegeti o optiune: ";
        cin >> choice;
        setConsoleColor(COLOR_DEFAULT | COLOR_BOLD);

        switch (choice) {
        case 1: {
            cout << "\nForma poloneza postfixata: ";
            setConsoleColor(COLOR_YELLOW | COLOR_BOLD);
            cout << postfix_r << endl;
            break;
        }
        case 2: {
            Node* root = buildSyntaxTree(postfix_r);
            setConsoleColor(COLOR_DEFAULT | COLOR_BOLD);
            cout << "\n--- Arbore Sintactic ---" << endl;
            if (root) printSyntaxTree(root);
            else cout << "Nu s-a putut construi arborele sintactic." << endl;
            setConsoleColor(COLOR_CYAN | COLOR_BOLD);
            break;
        }
        case 3: {
            cout << "\n--- AFD in Consola ---" << endl;
            AFD.printAutomaton(cout);
            ofstream outFile("out.txt");
            if (outFile.is_open()) {
                AFD.printAutomaton(outFile);
                outFile.close();
                cout << "AFD salvat in fisier" << endl;
            }
            else {
                cerr << "Eroare: Nu s-a putut deschide fisierul" << endl;
            }
            break;
        }
        case 4:
            cout << "Introduceti cuvantul de verificat: ";
            cin >> word_to_check;
            if (AFD.checkWord(word_to_check))
            {
                cout << "REZULTAT: Cuvantul este";
                setConsoleColor(COLOR_GREEN | COLOR_BOLD);
                cout << " ACCEPTAT ";
                setConsoleColor(COLOR_DEFAULT | COLOR_BOLD);
                cout << "de AFD." << endl;
            }
            else
            {
                cout << "REZULTAT: Cuvantul este";
                setConsoleColor(COLOR_RED | COLOR_BOLD);
                cout << " RESPINS ";
                setConsoleColor(COLOR_DEFAULT | COLOR_BOLD);
                cout << "de AFD." << endl;
                break;
            }
        case 0:
            cout << "Program incheiat" << endl;
            break;
        default:
            cout << "Optiune invalida. Reincercati" << endl;
            }
            setConsoleColor(COLOR_CYAN | COLOR_BOLD);
        } while (choice != 0);

        return 0;
    }
