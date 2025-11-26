#include "DeterministicFiniteAutomaton.h"
using namespace std;
using StateSet = set<int>;


DeterministicFiniteAutomaton RegexToDFA(const string& regex) {
    ;
}


// fct pt cerinta 3
string toPostfix(const string& regex) {
    ;
}

void printSyntaxTree(const string& regex) {
    ;
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
        regex_r = "a(a|b)*"; // Valoare implicită pentru testare
    }
    cout << "---------------------------------------" << endl;

    // Obtinerea AFD
    DeterministicFiniteAutomaton AFD = RegexToDFA(regex_r);
   
    if (!AFD.verifyAutomaton()) {
        cerr << "ATENTIE: Automat invalid!" << endl;
    }

    // Meniu
    int choice;
    string word_to_check;
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
            cout << "\nForma poloneza postfixata: " << toPostfix(regex_r) << endl;
            break;
        case 2:
            printSyntaxTree(regex_r);
            break;
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
