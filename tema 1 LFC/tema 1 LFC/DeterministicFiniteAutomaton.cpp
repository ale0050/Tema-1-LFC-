#include "DeterministicFiniteAutomaton.h"

void DeterministicFiniteAutomaton:: setQ(const set<int>& Q){ 
	Q_states = Q; 
}
void DeterministicFiniteAutomaton::setSigma(const set<char>& Sigma) { 
	Sigma_alphabet = Sigma; 
}
void DeterministicFiniteAutomaton:: setDelta(const map<pair<int, char>, int>& delta) { 
	delta_transition = delta; 
}
void DeterministicFiniteAutomaton::setQ0(int q0) { 
	q0_initialState = q0;
}
void DeterministicFiniteAutomaton::setF(const set<int>& F) { 
	F_finalStates = F; 
}


bool DeterministicFiniteAutomaton:: verifyAutomaton() const {
    // Starea inițială q0 aparține mulțimii Q
    if (Q_states.find(q0_initialState) == Q_states.end()) {
        cerr << "Eroare: Starea initiala nu apartine lui Q." << endl;
        return false;
    }

    // Toate stările finale F se află în Q
    for (int state : F_finalStates) {
        if (Q_states.find(state) == Q_states.end()) {
            cerr << "Eroare: Cel putin o stare finala nu apartine lui Q." << endl;
            return false;
        }
    }

    //Funcția de tranziție conține doar simboluri din Σ și stări din Q.

    for (const auto& entry : delta_transition) {
        int currentState = entry.first.first;
        char symbol = entry.first.second;
        int nextState = entry.second;

        if (Q_states.find(currentState) == Q_states.end() ||
            Q_states.find(nextState) == Q_states.end() ||
            Sigma_alphabet.find(symbol) == Sigma_alphabet.end()) {
            cerr << "Eroare: Tranzitie invalida gasita." << endl;
            return false;
        }
    }

    return true;
}

void DeterministicFiniteAutomaton::printAutomaton(ostream& os) const {
    os << "---------------------------------------" << endl;
    os << " AFD: M = (Q, Σ, δ, q0, F) " << endl;

    // Afișarea componentelor formale
    os << "Q (Stari): { ";
    for (int state : Q_states) os << state << " ";
    os << "}" << endl;

    os << "Σ (Alfabet): { ";
    for (char symbol : Sigma_alphabet) os << symbol << " ";
    os << "}" << endl;

    os << "q0 (Stare initiala): " << q0_initialState << endl;

    os << "F (Stari finale): { ";
    for (int state : F_finalStates) os << state << " ";
    os << "}" << endl;

    os << "---------------------------------------" << endl;

    // ** AFIȘAREA TABELULUI DE TRANZIȚIE **
    os << "--- Tabelul de Tranzitie (δ) ---" << endl;

    // Capul de tabel (Simbolurile)
    os << "Stare\\Simbol | ";
    for (char symbol : Sigma_alphabet) {
        os << symbol << " | ";
    }
    os << endl;

    // Linie separatoare
    os << "--------------";
    for (size_t i = 0; i < Sigma_alphabet.size(); ++i) os << "----";
    os << endl;

    // Corpul tabelului (Tranzițiile)
    for (int currentState : Q_states) {
        // Marcatori (-> pentru initiala, * pentru finala)
        if (currentState == q0_initialState) os << "->"; else os << "  ";
        if (F_finalStates.count(currentState)) os << "*"; else os << " ";

        os << currentState << " | ";

        // Căutarea tranzițiilor pentru fiecare simbol
        for (char symbol : Sigma_alphabet) {
            auto key = make_pair(currentState, symbol);

            if (delta_transition.count(key)) {
                // Afișează starea următoare
                os << delta_transition.at(key) << " | ";
            }
            else {
                os << "- | ";
            }
        }
        os << endl;
    }
    os << "---------------------------------------" << endl;
}

bool DeterministicFiniteAutomaton:: checkWord(const string& word) const {
    int currentState = q0_initialState;
    for (char symbol : word) {
        auto key = make_pair(currentState, symbol);
        // Verifică dacă există o tranziție pentru simbolul curent
        if (delta_transition.count(key)) {
            currentState = delta_transition.at(key);
        }
        else {
            // Nu există tranziție pentru acest simbol
            return false;
        }
    }
    // Verifică dacă starea finală este una dintre stările finale
    return F_finalStates.count(currentState) > 0;
}


