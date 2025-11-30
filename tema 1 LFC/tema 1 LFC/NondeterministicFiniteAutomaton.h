#pragma once
#include <set>
#include <map>
#include <stack>
#include <iostream>
#include "DeterministicFiniteAutomaton.h"

using namespace std;

constexpr char lambda = '\0';

class NondeterministicFiniteAutomaton
{
private:
	set<int> Q_states;
	set<char> Sigma_alphabet;
	map<pair<int, char>, set<int>> delta_transition;
	int q0_initialState;                           
	set<int> F_finalStates;

public: 
	static int next_state_index;

	NondeterministicFiniteAutomaton();

	void setQ(const set<int>& Q);
	void setSigma(const set<char>& Sigma);
	void setDelta(const map<pair<int, char>, set<int>>& delta);
	void setQ0(int q0);
	void setF(const set<int>& F);

	set<int> getQ() const;
	set<char> getSigma() const;
	int getQ0() const;
	const map<pair<int, char>, set<int>>& getDelta() const;

	void addState(int state);
	void addTransition(int from, char symbol, int to);
	void addSymbol(char symbol);
	void setInitialState(int state);
	void addFinalState(int state);

	set<int> lambdaClosure(const set<int>& states) const;
	set<int> move(const set<int>& states, char symbol) const;

	static NondeterministicFiniteAutomaton createBasicNFA(char symbol);

	NondeterministicFiniteAutomaton combineUnion(const NondeterministicFiniteAutomaton& other) const;     // Operator '|'
	NondeterministicFiniteAutomaton combineConcatenation(const NondeterministicFiniteAutomaton& other) const; // Operator '.'
	NondeterministicFiniteAutomaton combineKleeneStar() const;                                           // Operator '*'
	NondeterministicFiniteAutomaton combinePlus() const;                                                 // Operator '+'

	void printNFA(ostream& os) const;
	DeterministicFiniteAutomaton convertToDFA() const;


};

