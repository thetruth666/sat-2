#ifndef CLAUSE_H
#define CLAUSE_H
#include "common.h"
#include "myvector.h"
class Clause {
public:
	std::vector<Literal> literals;
	bool isSat = false;
	Clause() { literals.reserve(10); };
	/*Clause(std::vector<Literal> _literals, bool _status = false)
		:literals(_literals), isSat(_status) {}*/
	Clause &operator+=(Clause &_Clause) {
		for (auto i : _Clause.literals) {
			literals.push_back(i);
		}
		//literals.insert(literals.end(), _Clause.literals.begin(), _Clause.literals.end());
		isSat = false;
		return *this;
	}
};

#endif