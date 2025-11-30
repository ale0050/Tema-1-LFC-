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
    q0_initialState = -1; 
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
    Q_states.insert(state);
    q0_initialState = state;
}

void NondeterministicFiniteAutomaton::addFinalState(int state) {
    Q_states.insert(state);
    F_finalStates.insert(state);
}

void NondeterministicFiniteAutomaton::addSymbol(char symbol) {
    if (symbol != lambda)
        Sigma_alphabet.insert(symbol);
}

void NondeterministicFiniteAutomaton::addTransition(int from, char symbol, int to) {
    addState(from);
    addState(to);

    if (symbol != lambda)
        addSymbol(symbol);

    delta_transition[{from, symbol}].insert(to);
}


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

	//alocam inexi noi pt fiecare stare veche si le adaugam in result
    for (int oldState : allStates) {
        int newState = allocateStateIndex();
        mapping[oldState] = newState;
        result.addState(newState);
    }

	//copiem tranzitiile cu noile indici
    for (const auto& entry : other.getDelta()) {
        int oldFrom = entry.first.first;
        char symbol = entry.first.second;
        int newFrom = mapping.at(oldFrom);
        for (int oldTo : entry.second) {
            int newTo = mapping.at(oldTo);
            result.addTransition(newFrom, symbol, newTo);
        }
    }
	//copiem alfabetul fara lambda
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

    result.addTransition(iC, lambda, iA);
    result.addTransition(fA, lambda, fC);
    result.addTransition(fA, lambda, iA);

    return result;
}

NondeterministicFiniteAutomaton NondeterministicFiniteAutomaton::combineUnion(const NondeterministicFiniteAutomaton& other) const { //op |
    if (this->F_finalStates.size() != 1 || other.F_finalStates.size() != 1)
        throw std::runtime_error("Both NFAs must have exactly one final state for union.");

    NondeterministicFiniteAutomaton result;

    int newInitial = allocateStateIndex();
    int newFinal = allocateStateIndex();
    result.setInitialState(newInitial);
    result.addFinalState(newFinal);

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
    set<int> closure = states; 
    stack<int> s;              

    for (int state : states)
        s.push(state);

    while (!s.empty()) {
        int currentState = s.top();
        s.pop();

		//caut tranz curente pe lambda
        auto key = make_pair(currentState, lambda);

        if (delta_transition.count(key))
            //parcurg toate starile urmatoare
            for (int nextState : delta_transition.at(key)) 
                if (closure.find(nextState) == closure.end()) {
                    closure.insert(nextState);
                    s.push(nextState);        
        }
    }

    return closure;
}

set<int> NondeterministicFiniteAutomaton::move(const set<int>& states, char symbol) const {
	//calculam starile accesibile dintr-o multime de stari pe un simbol dat
    set<int> reachableStates;

    if (symbol == lambda)
        return reachableStates;

    for (int currentState : states) 
    {
        auto key = make_pair(currentState, symbol);
        if (delta_transition.count(key)) 
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
    int dfa_next_state_index = 0; 

    set<int> q0_nfa_set = { q0_initialState };
    set<int> q0_dfa_set = lambdaClosure(q0_nfa_set);

	// aduagam starea initiala in AFD
    dfa_states_map[q0_dfa_set] = dfa_next_state_index;
    dfa_q_states.insert(dfa_next_state_index);
    DFA.setQ0(dfa_next_state_index);
    dfa_next_state_index++;

	//stari de procesat
    queue<set<int>> states_to_process;
    states_to_process.push(q0_dfa_set);

    //procesam starile
    while (!states_to_process.empty()) 
    {
        set<int> current_nfa_set = states_to_process.front();
        states_to_process.pop();

        int current_dfa_state = dfa_states_map.at(current_nfa_set);

		//stările finale dfa sunt toate comb de stari nfa care includ cel putin o stare finala nfa
        for (int nfa_final_state : F_finalStates)
            if (current_nfa_set.count(nfa_final_state)) 
            {
                dfa_f_states.insert(current_dfa_state);
                break;
            }

       //calculam tranz pt fiecare elem din alfabet
        for (char symbol : Sigma_alphabet) 
        {
            set<int> target_nfa_set_after_move = move(current_nfa_set, symbol);
            set<int> target_dfa_set = lambdaClosure(target_nfa_set_after_move);

            if (target_dfa_set.empty())
                continue;

            int target_dfa_state;

			//verif daca noua sare a fota deja descoperita
            if (dfa_states_map.count(target_dfa_set))
                target_dfa_state = dfa_states_map.at(target_dfa_set);
            else {
                target_dfa_state = dfa_next_state_index;
                dfa_states_map[target_dfa_set] = target_dfa_state;
                dfa_q_states.insert(target_dfa_state);
                states_to_process.push(target_dfa_set);
                dfa_next_state_index++;
            }

            dfa_delta[{current_dfa_state, symbol}] = target_dfa_state;
        }
    }

    DFA.setQ(dfa_q_states);
    DFA.setSigma(Sigma_alphabet);
    DFA.setDelta(dfa_delta);
    DFA.setF(dfa_f_states);

    return DFA;
}

void NondeterministicFiniteAutomaton::printNFA(ostream& os) const {
    auto setToString = [](const set<int>& s) -> string {
        if (s.empty()) 
            return "{}";
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
    os << " AFN: M_lambda = (Q, sigma, delta, q0, F) " << endl;

    os << "Q (Stari): { ";
    for (int state : Q_states) 
        os << state << " ";
    os << "}" << endl;

    os << "sigma (Alfabet): { ";
    for (char symbol : Sigma_alphabet)
        os << symbol << " ";
    os << "}" << endl;

    os << "q0 (Stare initiala): " << q0_initialState << endl;

    os << "F (Stari finale): { ";
    for (int state : F_finalStates) 
        os << state << " ";
    os << "}" << endl;

    os << "---------------------------------------" << endl;

    os << "--- Tabelul de Tranzitie ---" << endl;

    os << "Stare\\Simbol | " << "lambda" << " | ";
    for (char symbol : Sigma_alphabet) {
        os << symbol << " | ";
    }
    os << endl;
    for (size_t i = 0; i < Sigma_alphabet.size() + 1; ++i) 
        os << "----";
    os << endl;

    for (int currentState : Q_states) 
    {
        if (currentState == q0_initialState) 
            os << "->"; else os << "  ";
        if (F_finalStates.count(currentState)) 
            os << "*"; else os << " ";

        os << currentState << " | ";

        //tranz lambda
        auto key_lambda = make_pair(currentState, lambda);
        if (delta_transition.count(key_lambda)) 
            os << setToString(delta_transition.at(key_lambda)) << " | ";
        else 
            os << "{} | ";
        
       
        // tranz pt fiecare simbol din alfabet
        for (char symbol : Sigma_alphabet) {
            auto key = make_pair(currentState, symbol);

            if (delta_transition.count(key))
                os << setToString(delta_transition.at(key)) << " | ";
            else
                os << "{} | ";
        }
        os << endl;
    }
    os << "=======================================" << endl;
}

