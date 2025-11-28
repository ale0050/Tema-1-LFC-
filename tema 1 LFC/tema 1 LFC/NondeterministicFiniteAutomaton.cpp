#include "NondeterministicFiniteAutomaton.h"
#include <algorithm>
#include <stdexcept>
#include <queue>

int NondeterministicFiniteAutomaton::next_state_index = 0;

NondeterministicFiniteAutomaton::NondeterministicFiniteAutomaton()
{
    Q_states.clear();
    Sigma_alphabet.clear();
    delta_transition.clear();
    q0_initialState = -1; // -1 indica o stare neinitializata
    F_finalStates.clear();
}

void NondeterministicFiniteAutomaton::setQ(const set<int>& Q) {
	Q_states = Q;
}

void NondeterministicFiniteAutomaton::setSigma(const set<char>& Sigma) {
	Sigma_alphabet = Sigma;
}

void NondeterministicFiniteAutomaton::setDelta(const map<pair<int, char>, set<int>>& delta) {
	delta_transition = delta;
}

void NondeterministicFiniteAutomaton::setQ0(int q0) {
	q0_initialState = q0;
}

void NondeterministicFiniteAutomaton::setF(const set<int>& F) {
	F_finalStates = F;
}

set<char> NondeterministicFiniteAutomaton::getSigma() const {
    // Alfabetul include doar simbolurile de intrare (nu EPSILON)
    return Sigma_alphabet;
}

int NondeterministicFiniteAutomaton::getQ0() const {
    return q0_initialState;
}

const map<pair<int, char>, set<int>>& NondeterministicFiniteAutomaton::getDelta() const {
    return delta_transition;
}

void NondeterministicFiniteAutomaton::addState(int state) {
    Q_states.insert(state);
}

void NondeterministicFiniteAutomaton::setInitialState(int state) {
    // Starea initiala trebuie sa fie inclusa si in multimea Q
    Q_states.insert(state);
    q0_initialState = state;
}

void NondeterministicFiniteAutomaton::addFinalState(int state) {
    // Starea finala trebuie sa fie inclusa si in multimea Q
    Q_states.insert(state);
    F_finalStates.insert(state);
}

void NondeterministicFiniteAutomaton::addSymbol(char symbol) {
    if (symbol != lambda)
        Sigma_alphabet.insert(symbol);
}

void NondeterministicFiniteAutomaton::addTransition(int from, char symbol, int to) {
    // Asigură-te că stările sunt adăugate în Q
    addState(from);
    addState(to);

    // Adaugă simbolul la alfabet (doar dacă nu e EPSILON)
    if (symbol != lambda) {
        addSymbol(symbol);
    }

    // Adaugă tranziția în harta delta
    delta_transition[{from, symbol}].insert(to);
}


NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::createBasicNFA(char symbol) {
    NondeterministicFiniteAutomaton nfa;

    // Starea inițială și finală primesc indexi noi de la contorul static
    int i = next_state_index;
    next_state_index++;
    int f = next_state_index;
    next_state_index++;

    nfa.setInitialState(i); // Folosim setInitialState, care adaugă starea în Q
    nfa.addFinalState(f);
    nfa.addTransition(i, symbol, f);
    // Alfabetul este adăugat automat de addTransition (dacă simbolul nu e LAMBDA)

    return nfa;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineConcatenation(const NondeterministicFiniteAutomaton& other) const { //nu am priceput asta inca & idk if it works also sa stergem comurile inaitne sa ii postam
 
    NondeterministicFiniteAutomaton result = *this; // Copiem NFA-ul A (rezultatul pana acum)

    if (result.F_finalStates.empty() || other.F_finalStates.empty()) 
        throw std::runtime_error("NFA must have exactly one final state for concatenation.");


    int fA = *result.F_finalStates.begin(); // Starea finală a lui A (devine starea contopită)
    int iB = other.q0_initialState;         // Starea inițială a lui B (cea care va fi contopită)

    // Stările din B sunt reindexate pentru a urma după ultima stare folosită în A (și contor)
    int offset = result.next_state_index;

    // Starea finală a lui A își pierde calitatea de stare finală
    result.F_finalStates.clear();

    // 5. Transferul tranzițiilor și stărilor din B, cu reindexare și contopire:
    for (const auto& entry : other.delta_transition) {
        int currentState = entry.first.first;
        char symbol = entry.first.second;

        // Starea de plecare reindexată (sau contopită)
        int fromState;
        if (currentState == iB) 
            fromState = fA; // iB se contopește cu fA
        else {
            fromState = currentState + offset;
            result.addState(fromState); // Adaugă și starea reindexată în Q
        }

        for (int nextState : entry.second) {
            // Starea de sosire reindexată (sau contopită)
            int toState;
            if (nextState == iB) {
                toState = fA; // Tranzițiile care intrau în iB intră acum în fA
            }
            else {
                toState = nextState + offset;
                result.addState(toState); // Adaugă și starea reindexată în Q
            }

            // Adaugă tranziția în NFA-ul rezultat
            result.addTransition(fromState, symbol, toState);
        }
    }

    // 6. Transferul simbolurilor din B
    for (char symbol : other.Sigma_alphabet) {
        result.addSymbol(symbol);
    }

    // 7. Actualizarea stării finale a rezultatului (care este fB reindexată)
    int fB_original = *other.F_finalStates.begin();

    int fB_new;
    if (fB_original == iB) {
        fB_new = fA; // Cazul special: fB este starea contopită (fA)
    }
    else {
        fB_new = fB_original + offset; // Starea finală reindexată
    }
    result.addFinalState(fB_new);

    // 8. Actualizarea contorului global (pentru următorul NFA care va fi creat)
    // Contorul se actualizează la valoarea maxima (offset + numarul total de stari din B)
    result.next_state_index = offset + other.Q_states.size();

    return result;
}


NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineKleeneStar() const {
    // A este 'this'
    NondeterministicFiniteAutomaton result = *this; // Copiem NFA-ul A

    // 1. Identificarea stărilor cheie vechi
    if (result.q0_initialState == -1 || result.F_finalStates.empty()) {
        // Tratează cazul NFA invalid
        throw std::runtime_error("NFA must be initialized for Kleene Star operation.");
    }

    int iA = result.q0_initialState;
    int fA = *result.F_finalStates.begin();

    // 2. Definirea noilor stări ($q_{contor}$ și $q_{contor+1}$)
    int offset = result.next_state_index; // Decalaj pentru noul NFA (care va fi C)

    int iC = offset;       // Noua stare inițială (q_contor) [cite: 83]
    int fC = offset + 1;   // Noua stare finală (q_contor+1) [cite: 84]

    // Contorul crește cu 2 [cite: 86]
    result.next_state_index = offset + 2;

    // 3. Adăugarea noilor stări în Q
    result.addState(iC);
    result.addState(fC);

    // 4. Stările vechi își pierd calitatea de inițială/finală
    result.F_finalStates.clear(); // fA își pierde calitatea de finală [cite: 84]
    // iA își pierde calitatea de inițială (dar iA rămâne în result.q0_initialState până la finalizarea construcției)

    // 5. Setarea noii stări inițiale și finale pentru C
    result.q0_initialState = iC; // q_contor devine noua stare inițială [cite: 83]
    result.addFinalState(fC);    // q_contor+1 devine noua stare finală [cite: 84]

    // 6. Adăugarea celor 4 λ-tranziții noi 

    // a) De la noua stare inițială la vechea stare iniţială: q_contor -> iA
    result.addTransition(iC, lambda, iA);

    // b) De la vechea stare finală la noua stare finală: fA -> q_contor+1
    result.addTransition(fA, lambda, fC);

    // c) De la vechea stare finală la vechea stare iniţială: fA -> iA (Buclă)
    result.addTransition(fA, lambda, iA);

    // d) De la noua stare iniţială la noua stare finală: q_contor -> q_contor+1 (Zero repetiții)
    result.addTransition(iC, lambda, fC);

    return result;
}


NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combinePlus() const {
    // A este 'this'
    NondeterministicFiniteAutomaton result = *this; // Copiem NFA-ul A

    // 1. Identificarea stărilor cheie vechi
    int iA = result.q0_initialState;
    int fA = *result.F_finalStates.begin();

    // 2. Definirea noilor stări ($q_{contor}$ și $q_{contor+1}$)
    int offset = result.next_state_index;

    int iC = offset;       // Noua stare inițială (q_contor)
    int fC = offset + 1;   // Noua stare finală (q_contor+1)

    // Contorul crește cu 2
    result.next_state_index = offset + 2;

    // 3. Adăugarea noilor stări și setarea noilor calități
    result.addState(iC);
    result.addState(fC);

    result.F_finalStates.clear();
    result.q0_initialState = iC; // q_contor devine noua stare inițială
    result.addFinalState(fC);    // q_contor+1 devine noua stare finală

    // 4. Adăugarea a 3 λ-tranziții (FĂRĂ q_contor -> q_contor+1)

    // a) De la noua stare inițială la vechea stare iniţială: q_contor -> iA
    result.addTransition(iC, lambda, iA);

    // b) De la vechea stare finală la noua stare finală: fA -> q_contor+1
    result.addTransition(fA, lambda, fC);

    // c) De la vechea stare finală la vechea stare iniţială: fA -> iA (Buclă)
    result.addTransition(fA, lambda, iA);

    return result;
}

// NondeterministicFiniteAutomaton.cpp

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineOptional() const {
    // A este 'this'
    NondeterministicFiniteAutomaton result = *this; // Copiem NFA-ul A

    // 1. Identificarea stărilor cheie vechi
    int iA = result.q0_initialState;
    int fA = *result.F_finalStates.begin();

    // 2. Definirea noilor stări ($q_{contor}$ și $q_{contor+1}$)
    int offset = result.next_state_index;

    int iC = offset;       // Noua stare inițială (q_contor)
    int fC = offset + 1;   // Noua stare finală (q_contor+1)

    // Contorul crește cu 2
    result.next_state_index = offset + 2;

    // 3. Adăugarea noilor stări și setarea noilor calități
    result.addState(iC);
    result.addState(fC);

    result.F_finalStates.clear();
    result.q0_initialState = iC; // q_contor devine noua stare inițială
    result.addFinalState(fC);    // q_contor+1 devine noua stare finală

    // 4. Adăugarea a 3 λ-tranziții (A | lambda)

    // a) De la noua stare inițială la vechea stare iniţială: q_contor -> iA (A se poate executa)
    result.addTransition(iC, lambda, iA);

    // b) De la vechea stare finală la noua stare finală: fA -> q_contor+1
    result.addTransition(fA, lambda, fC);

    // c) De la noua stare iniţială la noua stare finală: q_contor -> q_contor+1 (Lambda/Zero repetiții)
    result.addTransition(iC, lambda, fC);

    return result;
}

// NondeterministicFiniteAutomaton.cpp

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineUnion(const NondeterministicFiniteAutomaton& other) const {
    // A este 'this', B este 'other'
    NondeterministicFiniteAutomaton result;

    // 1. Identificarea stărilor cheie vechi
    int iA = q0_initialState;
    int fA = *F_finalStates.begin();
    int iB = other.q0_initialState;
    int fB = *other.F_finalStates.begin();

    // 2. Definirea noilor stări ($q_{contor}$ și $q_{contor+1}$)
    // Contorul global (next_state_index) se asigură de unicitate
    int offset = next_state_index;

    int iC = offset;       // Noua stare inițială (q_contor)
    int fC = offset + 1;   // Noua stare finală (q_contor+1)

    // Contorul global crește cu 2 (va fi actualizat la final)
    result.next_state_index = offset + 2;

    // 3. Setarea noii stări inițiale și finale
    result.setInitialState(iC);
    result.addFinalState(fC);

    // 4. Transferul stărilor, tranzițiilor și alfabetului din A
    for (int state : Q_states) {
        result.addState(state);
    }
    for (const auto& entry : delta_transition) {
        for (int toState : entry.second) {
            result.addTransition(entry.first.first, entry.first.second, toState);
        }
    }
    for (char symbol : Sigma_alphabet) {
        result.addSymbol(symbol);
    }

    // 5. Calculul decalajului (offset) pentru stările din B
    // Stările lui B trebuie reindexate pentru a nu se suprapune cu stările din A
    // Offset-ul trebuie sa fie cel putin ultima stare din A + 1.
    // Deoarece NFA-urile sunt construite secvential, folosim contorul de la care a plecat NFA-ul A.
    int offset_B = result.next_state_index;

    // 6. Transferul stărilor și tranzițiilor lui B (cu offset_B)
    for (int state : other.Q_states) {
        result.addState(state + offset_B);
    }
    for (const auto& entry : other.delta_transition) {
        for (int toState : entry.second) {
            result.addTransition(entry.first.first + offset_B, entry.first.second, toState + offset_B);
        }
    }
    for (char symbol : other.Sigma_alphabet) {
        result.addSymbol(symbol);
    }


    // 7. Adăugarea celor 4 λ-tranziții [cite: 70]

    // a) De la noua stare inițială la fosta stare inițială a lui A: q_contor -> iA
    result.addTransition(iC, lambda, iA);

    // b) De la noua stare inițială la fosta stare inițială a lui B: q_contor -> iB (reindexat)
    result.addTransition(iC, lambda, iB + offset_B);

    // c) De la fosta stare finală a lui A către noua stare finală: fA -> q_contor+1
    result.addTransition(fA, lambda, fC);

    // d) De la fosta stare finală a lui B (reindexată) către noua stare finală: fB -> q_contor+1
    result.addTransition(fB + offset_B, lambda, fC);

    // 8. Actualizarea contorului global
    // Contorul se actualizează la cea mai mare stare folosită + 1.
    // Cea mai mare stare a fost max(fC, max_state_in_B_reindexed)
    int max_state_B = *other.Q_states.rbegin() + offset_B;
    result.next_state_index = max(fC, max_state_B) + 1;


    return result;
}

// NondeterministicFiniteAutomaton.cpp

set<int> NondeterministicFiniteAutomaton::lambdaClosure(const set<int>& states) const {
    set<int> closure = states; // 1. Setul de rezultat incepe cu starile de intrare
    stack<int> s;              // Structura auxiliara (stiva) pentru DFS

    // Incarcam toate starile initiale in stiva
    for (int state : states)
        s.push(state);

    while (!s.empty()) {
        int currentState = s.top();
        s.pop();

        // Cautam tranzitiile din currentState pe simbolul EPSILON
        // Accesam delta_transition direct cu cheia {currentState, EPSILON}
        auto key = make_pair(currentState, lambda);

        // Verificam daca exista tranzitii pentru EPSILON
        if (delta_transition.count(key)) {
            // Parcurgem toate starile urmatoare (set<int>)
            for (int nextState : delta_transition.at(key)) {

                // Daca nextState nu a fost vizitata (nu e in setul closure)
                if (closure.find(nextState) == closure.end()) {
                    closure.insert(nextState); // Adaugam la multimea de rezultat
                    s.push(nextState);         // Adaugam in stiva pentru procesare ulterioara
                }
            }
        }
    }

    return closure;
}

set<int> NondeterministicFiniteAutomaton::move(const set<int>& states, char symbol) const {
    set<int> reachableStates; // Mulțimea de stări atinse

    // Nu ar trebui să se încerce move pe simbolul LAMBDA
    if (symbol == lambda) {
        return reachableStates;
    }

    // Parcurgem toate stările din mulțimea de intrare
    for (int currentState : states) {
        // Căutăm tranzițiile din currentState pe simbolul de intrare
        auto key = make_pair(currentState, symbol);

        // Verificăm dacă există tranziții pentru simbolul dat
        if (delta_transition.count(key)) {
            // Dacă există, adăugăm toate stările următoare la mulțimea de rezultat
            for (int nextState : delta_transition.at(key)) {
                reachableStates.insert(nextState);
            }
        }
    }

    return reachableStates;
}

// NondeterministicFiniteAutomaton.cpp

DeterministicFiniteAutomaton NondeterministicFiniteAutomaton::convertToDFA() const {
    DeterministicFiniteAutomaton DFA;

    // 1. Initializare
    // Stările AFD sunt mulțimi de stări AFN (set<int>).
    // Folosim o hartă pentru a mapa fiecare mulțime unică de stări AFN la un index de stare DFA (int).
    map<set<int>, int> dfa_states_map;
    set<int> dfa_q_states;
    map<pair<int, char>, int> dfa_delta;
    set<int> dfa_f_states;
    int dfa_next_state_index = 0; // Contorul pentru stările DFA (0, 1, 2, ...)

    // a) Starea inițială a DFA: q0' = lambdaClosure({q0_AFN})
    set<int> q0_nfa_set = { q0_initialState };
    set<int> q0_dfa_set = lambdaClosure(q0_nfa_set);

        // b) Adaugă starea inițială la mulțimile AFD
    dfa_states_map[q0_dfa_set] = dfa_next_state_index;
    dfa_q_states.insert(dfa_next_state_index);
    DFA.setQ0(dfa_next_state_index);
    dfa_next_state_index++;

    // c) Stări de procesat (cele pentru care nu s-au calculat încă toate tranzițiile)
    queue<set<int>> states_to_process;
    states_to_process.push(q0_dfa_set);

    // 2. Procesarea stărilor (Construcția Submulțimilor)
    while (!states_to_process.empty()) {
        set<int> current_nfa_set = states_to_process.front();
        states_to_process.pop();

        int current_dfa_state = dfa_states_map.at(current_nfa_set);

        // 3. Verificare Stare Finală
        // Dacă mulțimea AFN (current_nfa_set) conține orice stare finală AFN, 
        // atunci starea AFD curentă (current_dfa_state) este finală. 
        for (int nfa_final_state : F_finalStates) {
            if (current_nfa_set.count(nfa_final_state)) {
                dfa_f_states.insert(current_dfa_state);
                break;
            }
        }

        // 4. Calcularea tranzițiilor pentru fiecare simbol din alfabet
        for (char symbol : Sigma_alphabet) {

            // a) Mutare: move(T, a)
            set<int> target_nfa_set_after_move = move(current_nfa_set, symbol);

            // b) Închidere: lambdaClosure(move(T, a))
            set<int> target_dfa_set = lambdaClosure(target_nfa_set_after_move);

            // Dacă mulțimea rezultată este vidă, ignorăm tranziția (sau putem adăuga o stare capcană)
            if (target_dfa_set.empty()) {
                continue;
            }

            int target_dfa_state;

            // c) Verifică dacă noua stare AFD (target_dfa_set) a fost deja descoperită
            if (dfa_states_map.count(target_dfa_set)) {
                // Stare existentă
                target_dfa_state = dfa_states_map.at(target_dfa_set);
            }
            else {
                // Stare nouă (Descoperită)
                target_dfa_state = dfa_next_state_index;
                dfa_states_map[target_dfa_set] = target_dfa_state;
                dfa_q_states.insert(target_dfa_state);
                states_to_process.push(target_dfa_set);
                dfa_next_state_index++;
            }

            // d) Adaugă tranziția la AFD
            dfa_delta[{current_dfa_state, symbol}] = target_dfa_state;
        }
    }

    // 5. Setarea componentelor AFD finale
    DFA.setQ(dfa_q_states);
    DFA.setSigma(Sigma_alphabet);
    DFA.setDelta(dfa_delta);
    DFA.setF(dfa_f_states);

    return DFA;
}

void NondeterministicFiniteAutomaton::printNFA(ostream& os) const {
    // Helper local: convertește set<int> în string "{1,2,3}"
    auto setToString = [](const set<int>& s) -> string {
        if (s.empty()) return "{}";
        string out = "{";
        bool first = true;
        for (int st : s) {
            if (!first) out += ",";
            out += to_string(st);
            first = false;
        }
        out += "}";
        return out;
        };

    os << "=======================================" << endl;
    os << " AFN: M_lambda = (Q, \u03A3, \u03B4, q0, F) " << endl;

    // 1. Afișare componente formale
    os << "Q (Stari): { ";
    for (int state : Q_states) os << state << " ";
    os << "}" << endl;

    os << "\u03A3 (Alfabet): { ";
    for (char symbol : Sigma_alphabet) os << symbol << " ";
    os << "}" << endl;

    os << "q0 (Stare initiala): " << q0_initialState << endl;

    os << "F (Stari finale): { ";
    for (int state : F_finalStates) os << state << " ";
    os << "}" << endl;

    os << "---------------------------------------" << endl;

    // 2. Afisare Tabel de Tranzitie (incluzând λ)
    os << "--- Tabelul de Tranzitie (\u03B4) ---" << endl;

    // Header: Simboluri: Alfabet + LAMBDA
    os << "Stare\\Simbol | " << "\u03BB (LAMBDA)" << " | ";
    for (char symbol : Sigma_alphabet) {
        os << symbol << " | ";
    }
    os << endl;

    // Linie separatoare (doar estetic)
    os << "--------------";
    for (size_t i = 0; i < Sigma_alphabet.size() + 1; ++i) os << "----";
    os << endl;

    // Tranziții
    for (int currentState : Q_states) {
        // Marcaje: -> pentru initiala, * pentru finala
        if (currentState == q0_initialState) os << "->"; else os << "  ";
        if (F_finalStates.count(currentState)) os << "*"; else os << " ";

        os << currentState << " | ";

        // Tranzitie LAMBDA
        auto key_lambda = make_pair(currentState, lambda);
        if (delta_transition.count(key_lambda)) {
            os << setToString(delta_transition.at(key_lambda)) << " | ";
        }
        else {
            os << "{} | ";
        }

        // Tranzițiile pentru fiecare simbol din alfabet
        for (char symbol : Sigma_alphabet) {
            auto key = make_pair(currentState, symbol);

            if (delta_transition.count(key)) {
                os << setToString(delta_transition.at(key)) << " | ";
            }
            else {
                os << "{} | ";
            }
        }
        os << endl;
    }
    os << "=======================================" << endl;
}

