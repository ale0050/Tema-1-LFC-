#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
using namespace std;


class DeterministicFiniteAutomaton
{
private:
    set<int> Q_states;                           // Q - Mulțimea stărilor
    set<char> Sigma_alphabet;                    // Sigma - Alfabetul de intrare
    map<pair<int, char>, int> delta_transition;  // delta - Funcția de tranziție (stare, simbol) -> stare nouă
    int q0_initialState;                         // q0 - Starea inițială
    set<int> F_finalStates;                      // F - Mulțimea stărilor finale

public: 
    // setteri
    void setQ(const set<int>& Q);
    void setSigma(const set<char>& Sigma);
    void setDelta(const map<pair<int, char>, int>& delta);
    void setQ0(int q0);
    void setF(const set<int>& F);

	// verif daca e automat valid
    bool verifyAutomaton() const; 
    void printAutomaton(ostream& os) const;  
	bool checkWord(const string& word) const;
};

