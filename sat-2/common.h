#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <ctime>
#include <stack>
#include <queue>
#include "myvector.h"

enum Status {
	unknown,       //δָ��
	satisfied,     //����
	unsatisfied    //��������
};
struct Literal {
	int var;
	int sign;    //�������� -1������ 1������


	Literal(int var = 0, int sign = 0) : var(var), sign(sign) {}
	bool operator==(const Literal &_literal) {
		return (var == _literal.var && sign == _literal.sign);
	}
};

struct cls_lit {
	int cls_idx;
	Literal lit;
};

struct Var {
	int value = 0;                  //-1�� 0δ��ֵ 1 ��
	int dlevel = -1;
	int antecedent = -1;
	std::vector<std::pair<int, int> > var_clauses;   // <int:�Ӿ�pos bool:��������>

	int frequency = 0;
	int back_frequency = 0;
	int polarity = 0;
};

#define MAX_BUFFER_SIZE 5000000
#define SAT_BUFFER_SIZE 50000
#define UNSAT_BUFFER_SIZE 100
#define UNKNOWN_BUFFER_SIZE 100
#endif