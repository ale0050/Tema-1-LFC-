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

set<int> NondeterministicFiniteAutomaton::getQ() const
{
    return Q_states;
}

set<char> NondeterministicFiniteAutomaton::getSigma() const {
    // includem doar simbolurile de intrare, fara lambda
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
    // adaugam starule in Q
    addState(from);
    addState(to);

    //adaugam simbolul la alfabet doar daca nu e lambda
    if (symbol != lambda)
        addSymbol(symbol);

    // Adaugă tranziția în harta delta
    delta_transition[{from, symbol}].insert(to);
}


//alocam index nou pt fiecare stare noua 
static int allocateStateIndex() {
    return NondeterministicFiniteAutomaton::next_state_index++;
}

static map<int, int> mergeWithOffset(NondeterministicFiniteAutomaton& result, const NondeterministicFiniteAutomaton& other) {
    map<int, int> mapping;
    set<int> allStates;

    //Colectăm toate stările: cele din getQ() și cele care apar în getDelta()
    for (int s : other.getQ())
        allStates.insert(s);

    for (const auto& entry : other.getDelta()) {
        int from = entry.first.first;
        allStates.insert(from);
        for (int to : entry.second) allStates.insert(to);
    }

    //Alocăm indecși noi pentru fiecare stare din other
    for (int oldState : allStates) {
        int newState = allocateStateIndex();
        mapping[oldState] = newState;
        result.addState(newState);
    }

    //Copiem tranzițiile cu reindexare
    for (const auto& entry : other.getDelta()) {
        int oldFrom = entry.first.first;
        char symbol = entry.first.second;
        int newFrom = mapping.at(oldFrom);
        for (int oldTo : entry.second) {
            int newTo = mapping.at(oldTo);
            result.addTransition(newFrom, symbol, newTo);
        }
    }
    //Copiem alfabetul (fără lambda)
    for (char s : other.getSigma()) result.addSymbol(s);

    return mapping;
}



NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::createBasicNFA(char symbol) {
    NondeterministicFiniteAutomaton nfa;
    int i = allocateStateIndex();
    int f = allocateStateIndex();
    nfa.setInitialState(i);
    nfa.addFinalState(f);
    nfa.addTransition(i, symbol, f);
    return nfa;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineConcatenation(const NondeterministicFiniteAutomaton& other) const { //leaga starea finala a primului nfa la starea init a urmat nfa  op .
   
    //validam starea finala pt ambele
    if (this->F_finalStates.size() != 1 || other.F_finalStates.size() != 1)
        throw std::runtime_error("Both NFAs must have exactly one final state for concatenation.");

    //punem primul nfa in rez final
    NondeterministicFiniteAutomaton result = *this;

    //pt nfa 2 obitne indici noi pt stari
    map<int, int> mapping = mergeWithOffset(result, other);

    //pregatesc starile fA si iB ca sa le leg 
    int fA_old = *this->F_finalStates.begin();
    int fA_new = fA_old;

    int iB_old = other.q0_initialState;
    int iB_new = mapping.at(iB_old);

   //eliminam starea finala a lui a deoarece se lipseste cu b 
    result.F_finalStates.clear();

    //adaugam lambda tranzitie inte a si b
    result.addTransition(fA_new, lambda, iB_new);

    //luam starea finala a lui b si o facem stare finala pt result
    int fB_old = *other.F_finalStates.begin();
    int fB_new = mapping.at(fB_old);
    result.addFinalState(fB_new);

    return result;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineKleeneStar() const { //op *
    if (this->q0_initialState == -1 || this->F_finalStates.empty())
        throw std::runtime_error("NFA must be initialized for Kleene Star operation.");

    NondeterministicFiniteAutomaton result = *this; // copia A

    int iA = this->q0_initialState;
    int fA = *this->F_finalStates.begin();

    // Alocam doua stari noi clar si sigur
    int iC = allocateStateIndex();
    int fC = allocateStateIndex();

    result.addState(iC);
    result.addState(fC);

    // fA își pierde calitatea de finală
    result.F_finalStates.clear();

    // Setam noua stare initiala si finala
    result.setInitialState(iC);
    result.addFinalState(fC);

    // Adaugam cele 4 tranzitii lambda
    result.addTransition(iC, lambda, iA);
    result.addTransition(iC, lambda, fC);
    result.addTransition(fA, lambda, iA);
    result.addTransition(fA, lambda, fC);

    return result;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combinePlus() const { //op +
    if (this->q0_initialState == -1 || this->F_finalStates.empty())
        throw std::runtime_error("NFA must be initialized for Plus operation.");

    NondeterministicFiniteAutomaton result = *this;

    int iA = this->q0_initialState;
    int fA = *this->F_finalStates.begin();

    int iC = allocateStateIndex();
    int fC = allocateStateIndex();

    result.addState(iC);
    result.addState(fC);

    result.F_finalStates.clear();
    result.setInitialState(iC);
    result.addFinalState(fC);

    // 3 lambda (fara iC -> fC)
    result.addTransition(iC, lambda, iA);
    result.addTransition(fA, lambda, fC);
    result.addTransition(fA, lambda, iA);

    return result;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineOptional() const { // op ?
    //verif daca nfa curent e valid
    if (this->q0_initialState == -1 || this->F_finalStates.empty())
        throw std::runtime_error("NFA must be initialized for Optional operation.");

    NondeterministicFiniteAutomaton result = *this;

    //retine starea f si i ale nfa ului
    int iA = this->q0_initialState;
    int fA = *this->F_finalStates.begin();

    //cream stari f si i noi
    int iC = allocateStateIndex();
    int fC = allocateStateIndex();

    result.addState(iC);
    result.addState(fC);

    //setam starile f si i pt nfa ul final
    result.F_finalStates.clear();
    result.setInitialState(iC);
    result.addFinalState(fC);

    // lambda: iC -> iA, fA -> fC, iC -> fC
    result.addTransition(iC, lambda, iA);
    result.addTransition(fA, lambda, fC);
    result.addTransition(iC, lambda, fC);

    return result;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineUnion(const NondeterministicFiniteAutomaton& other) const { //op |
    //ambele trebuie sa aiba fix o stare fianla
    if (this->F_finalStates.size() != 1 || other.F_finalStates.size() != 1)
        throw std::runtime_error("Both NFAs must have exactly one final state for union.");

    NondeterministicFiniteAutomaton result;

    //alocam si setam doua stari noi
    int newInitial = allocateStateIndex();
    int newFinal = allocateStateIndex();
    result.setInitialState(newInitial);
    result.addFinalState(newFinal);

    //copie nfa ul curent si urmat in  result
    map<int, int> mappingA = mergeWithOffset(result, *this);
    map<int, int> mappingB = mergeWithOffset(result, other);

    //obtinem starile initiale reindexate ale celor doua nfa uri si pt
    //fiecare facem tranzitii lambda de la noua strae init la
    //starile init din celelalte ai union sa inceapa dintr-un singur loc
    int iA_mapped = mappingA.at(this->q0_initialState);
    int iB_mapped = mappingB.at(other.q0_initialState);
    result.addTransition(newInitial, lambda, iA_mapped);
    result.addTransition(newInitial, lambda, iB_mapped);


    //ca la starile initiale doar ca pt starea finala
    int fA_mapped = mappingA.at(*this->F_finalStates.begin());
    int fB_mapped = mappingB.at(*other.F_finalStates.begin());
    result.addTransition(fA_mapped, lambda, newFinal);
    result.addTransition(fB_mapped, lambda, newFinal);

    return result;
}


set<int> NondeterministicFiniteAutomaton::lambdaClosure(const set<int>& states) const {
    set<int> closure = states; // Setul de rezultat incepe cu starile de intrare
    stack<int> s;              // Structura auxiliara (stiva) pentru DFS

    // Incarcam toate starile initiale in stiva
    for (int state : states)
        s.push(state);

    while (!s.empty()) {
        int currentState = s.top();
        s.pop();

        // Cautam tranzitiile din currentState pe simbolul lambda
        // Accesam delta_transition direct cu cheia {currentState, lambda}
        auto key = make_pair(currentState, lambda);

        // Verificam daca exista tranzitii pentru lambda
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
    if (symbol == lambda)
        return reachableStates;

    // Parcurgem toate stările din mulțimea de intrare
    for (int currentState : states) {
        // Căutăm tranzițiile din currentState pe simbolul de intrare
        auto key = make_pair(currentState, symbol);

        // Verificăm dacă există tranziții pentru simbolul dat
        if (delta_transition.count(key)) 
            // Dacă există, adăugăm toate stările următoare la mulțimea de rezultat
            for (int nextState : delta_transition.at(key))
                reachableStates.insert(nextState);
         
    }

    return reachableStates;
}


DeterministicFiniteAutomaton NondeterministicFiniteAutomaton::convertToDFA() const {
    DeterministicFiniteAutomaton DFA;

    if (q0_initialState == -1) 
        throw std::runtime_error("ConvertToDFA error: stare initiala neinitializata (q0 = -1).");
    if (Sigma_alphabet.empty())
        throw std::runtime_error("ConvertToDFA error: alfabet (Sigma) este gol.");

    map<set<int>, int> dfa_states_map;
    set<int> dfa_q_states;
    map<pair<int, char>, int> dfa_delta;
    set<int> dfa_f_states;
    int dfa_next_state_index = 0; // Contorul pentru stările DFA (0, 1, 2, ...)

    // Starea inițială a DFA: q0' = lambdaClosure({q0_AFN})
    set<int> q0_nfa_set = { q0_initialState };
    set<int> q0_dfa_set = lambdaClosure(q0_nfa_set);

    // Adaugă starea inițială la mulțimile AFD
    dfa_states_map[q0_dfa_set] = dfa_next_state_index;
    dfa_q_states.insert(dfa_next_state_index);
    DFA.setQ0(dfa_next_state_index);
    dfa_next_state_index++;

    // Stări de procesat (cele pentru care nu s-au calculat încă toate tranzițiile)
    queue<set<int>> states_to_process;
    states_to_process.push(q0_dfa_set);

    //procesam starile
    while (!states_to_process.empty()) 
    {
        set<int> current_nfa_set = states_to_process.front();
        states_to_process.pop();

        int current_dfa_state = dfa_states_map.at(current_nfa_set);

        // verificam starea finala
        // Dacă mulțimea AFN (current_nfa_set) conține orice stare finală AFN, 
        // atunci starea AFD curentă (current_dfa_state) este finală. 
        for (int nfa_final_state : F_finalStates) {
            if (current_nfa_set.count(nfa_final_state)) {
                dfa_f_states.insert(current_dfa_state);
                break;
            }
        }

       //calculam tranzitiile pt fiecare elem din alfabet
        for (char symbol : Sigma_alphabet) 
        {

            // Mutare: move(T, a)
            set<int> target_nfa_set_after_move = move(current_nfa_set, symbol);

            // Închidere: lambdaClosure(move(T, a))
            set<int> target_dfa_set = lambdaClosure(target_nfa_set_after_move);

            // daca avem mult vida ignoram tranzitia
            if (target_dfa_set.empty())
                continue;

            int target_dfa_state;

            // Verifică dacă noua stare AFD (target_dfa_set) a fost deja descoperită
            if (dfa_states_map.count(target_dfa_set))
                // Stare existentă
                target_dfa_state = dfa_states_map.at(target_dfa_set);
            else {
                // Stare nouă (Descoperită)
                target_dfa_state = dfa_next_state_index;
                dfa_states_map[target_dfa_set] = target_dfa_state;
                dfa_q_states.insert(target_dfa_state);
                states_to_process.push(target_dfa_set);
                dfa_next_state_index++;
            }

            //Adaugă tranziția la AFD
            dfa_delta[{current_dfa_state, symbol}] = target_dfa_state;
        }
    }

    //setam comp afd finale
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

