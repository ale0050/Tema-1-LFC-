#include "DeterministicFiniteAutomaton.h"
#include "NondeterministicFiniteAutomaton.h"
#include <iostream>
#include <stack>
#include <windows.h>


// Obține un handle pentru ieșirea standard
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// Culori standard Windows  ---> idk daca merg 
#define COLOR_DEFAULT 7  
#define COLOR_BLUE 9
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_RED 12
#define COLOR_CYAN 11
#define COLOR_BOLD 8

using namespace std;

struct Node { // --> pt syntax tree idk daca e bun 
    char value;    // operator sau simbol
    Node* left;    // pentru operatori binari
    Node* right;   // pentru operatori binari
    // pentru operatori unari se folosește doar left
    Node(char v, Node* l = nullptr, Node* r = nullptr)
        : value(v), left(l), right(r) {
    }
};

bool isOperand(char c)
{
    return isalnum((unsigned char)c);
}

bool isUnaryOperator(char c)
{
    return c == '*' || c == '+' || c == '?';

}

string insertConcatenation(const string& regex)
{
    string processedRegex = "";
    for (size_t i = 0; i < regex.length(); ++i)
    {
        char currentChar = regex[i];
        processedRegex += currentChar;

        if (i + 1 < regex.length())
        {
            char nextChar = regex[i + 1];
            bool leftRequired = isOperand(currentChar) || currentChar == ')' || isUnaryOperator(currentChar);
            bool rightRequired = isOperand(nextChar) || nextChar == '(';
            if (leftRequired && rightRequired)
                processedRegex += '.';
        }
    }
    return processedRegex;
}

void setConsoleColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}


int priority(char op) { // e buna 
    switch (op) 
    {
    case '*': case '+': case '?': return 3; // cele mai imp
    case '.': return 2;                    
    case '|': return 1;                   
    default: return 0;
    }
}



NondeterministicFiniteAutomaton regexToNFA_thompson(const string& postfix) {
    stack<NondeterministicFiniteAutomaton> nfaStack;

    for (char c : postfix) {
        if (isalnum((unsigned char)c)) 
            nfaStack.push(NondeterministicFiniteAutomaton::createBasicNFA(c));
        else if (c == '.') 
        {
            
            NondeterministicFiniteAutomaton NFA2 = nfaStack.top(); nfaStack.pop();
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();

            nfaStack.push(NFA1.combineConcatenation(NFA2));
        }
        else if (c == '|') {
            // Cazul Uniune: A|B
            NondeterministicFiniteAutomaton NFA2 = nfaStack.top(); nfaStack.pop();
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();

            nfaStack.push(NFA1.combineUnion(NFA2));
        }
        else if (c == '*') {
            // Cazul Kleene Star: A*
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combineKleeneStar());
        }
        else if (c == '+') {
            // Cazul Plus (una sau mai multe): A+
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combinePlus());
        }
        else if (c == '?') {
            // Cazul Optional: A?
            NondeterministicFiniteAutomaton NFA1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(NFA1.combineOptional());
        }
      
    }

    if (nfaStack.empty()) {
        throw std::runtime_error("Expresie regulata invalida sau goala.");
    }

    return nfaStack.top(); // NFA-ul final
}


// fct pt cerinta 3
string toPostfix(const string& regex) { // e verificata si e buna 
        stack<char> operators;     // stiva operatorilor
        string out;         // output postfix

        for (char c : regex) {
           // verif daca e nr sau litera
            if (isOperand(c)) 
                out += c;
      
            // ( - punem pe stivă
            else if (c == '(') {
                operators.push(c);
            }
            // ) - scoatem pana la (
            else if (c == ')') {
                while (!operators.empty() && operators.top() != '(') {
                    out += operators.top();
                    operators.pop();
                }
                if (!operators.empty() && operators.top() == '(') operators.pop(); // scoatem (
            }
            // daca e operator: Shunting Yard 
            else {
				//scoatem toti op cu prioritate mai mare\ egala 
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

DeterministicFiniteAutomaton RegexToDFA(const string& regex)
{
    string processed_regex = insertConcatenation(regex);

    string postfix_r = toPostfix(processed_regex);

    NondeterministicFiniteAutomaton NFA = regexToNFA_thompson(postfix_r);

    return NFA.convertToDFA();
}

Node* buildSyntaxTree(const string& postfix) { // -> we don t know this
    stack<Node*> st;

    for (char c : postfix) {
        if (isalnum(c)) {
            st.push(new Node(c));
        }
        else if (c == '*' || c == '+' || c == '?') {
            Node* a = st.top(); st.pop();
            st.push(new Node(c, a));
        }
        else if (c == '.' || c == '|') {
            Node* b = st.top(); st.pop();
            Node* a = st.top(); st.pop();
            st.push(new Node(c, a, b));
        }
    }

    return st.top();
}


void printSyntaxTree(Node* root, string indent = "", bool last = true) { // we don t know this either 
    if (!root) return;

    cout << indent;
    if (last) {
        cout << "+--";
        indent += "   ";
    }
    else {
        cout << "|--";
        indent += "|  ";
    }

    cout << root->value << "\n";

    if (root->left)  printSyntaxTree(root->left, indent, root->right == nullptr);
    if (root->right) printSyntaxTree(root->right, indent, true);
}





int main() {

    //citire expresie din fisier
    ifstream inFile("regexInput.txt");
    string regex_r;
 

    if (inFile.is_open()) {
        getline(inFile, regex_r);
        inFile.close();
        cout << "Expresia regulata citita: " << regex_r << endl;
    }
    else {
        cerr << "Eroare: Nu s-a putut deschide fisierul. Val implicita: " << endl;
        regex_r = "a(a|b)*"; // Valoare implicita pentru testare
    }
    cout << "---------------------------------------" << endl;

    string processed_regex = insertConcatenation(regex_r);
    string postfix_r = toPostfix(processed_regex);
    
    //NondeterministicFiniteAutomaton NFA = regexToNFA_thompson(postfix_r);
    //cout << "\n--- AFN in Consola (Rezultat Thompson) ---" << endl;
    //NFA.printNFA(cout);
    //DeterministicFiniteAutomaton AFD = NFA.convertToDFA();
   
    DeterministicFiniteAutomaton AFD = RegexToDFA(regex_r);
   
    if (!AFD.verifyAutomaton()) {
        cerr << "ATENTIE: Automat invalid!" << endl;
    }

    // Meniu
    int choice;
    string word_to_check;
    setConsoleColor(COLOR_BLUE | COLOR_BOLD);
    do {
        cout << "\n--- MENIU AFD ---" << endl;
        cout << "1. Afisare forma poloneza postfixata (NPI)" << endl;
        cout << "2. Afisare arbore sintactic" << endl;
        cout << "3. Afisare automat in consola si fisier" << endl;
        cout << "4. Verificare cuvant in automat " << endl;
        cout << "0. Iesire" << endl;
        cout << "Alegeti o optiune: ";
        cin >> choice;

        switch (choice) {
        case 1:
            cout << "\nForma poloneza postfixata: " << toPostfix(processed_regex) << endl;
            break;
        case 2:
        {
            //string processed_regex = insertConcatenation(regex_r);
            //string postfix_r = toPostfix(processed_regex);
            Node* root = buildSyntaxTree(postfix_r);
            setConsoleColor(COLOR_RED | COLOR_BOLD);
            cout << "\n--- Arbore Sintactic ---" << endl;

            if (root) {
                printSyntaxTree(root);
            }
            else {
                cout << "Nu s-a putut construi arborele sintactic." << endl;
            }

            setConsoleColor(COLOR_BLUE | COLOR_BOLD);
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
            if (AFD.checkWord(word_to_check)) {
                cout << "REZULTAT: Cuvantul este ACCEPTAT de AFD." << endl;
            }
            else {
                cout << "REZULTAT: Cuvantul este RESPINS de AFD." << endl;
            }
            break;
        case 0:
            cout << "Program incheiat." << endl;
            break;
        default:
            cout << "Optiune invalida. Reincercati." << endl;
        }
    } while (choice != 0);

    return 0;
}
