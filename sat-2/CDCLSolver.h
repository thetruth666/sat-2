#ifndef CDCLSOLVER_H
#define CDCLSOLVER_H

#include "common.h"
#include "Clause.h"
#include "myvector.h"
using namespace std;
class CDCLSolver {
private:
	std::vector<Var> vars;
	std::vector<Clause> clauses;
	int nvar;
	int nclause;
	int antecedent_conflict;
	int nAssigned;
	int pick_counter;
	int result;
	int probability = 3;
	int conflict_cnt = 0;
	bool isRand = false;
	double time_diff;

	std::random_device random_generator;
	std::mt19937 generator;

	void assign(Literal _literal, int antecedent, int dlevel);
	void undo_assign(int _var);
	//BCP
	void CDCL(int time_limit);
	cls_lit search_unit();
	int begin_propagate();
	int unit_propagate(int _dlevel, Literal lit, int antecedent);

	//子句学习和回溯函数
	int conflict_analysis(int _dlevel);
	Clause &resolve(Clause &learn_clause, int var);
	int back_jump(int back_dlevel);
	//选取分支变量
	Literal pick_branch_var();
	//文件的读取与输出
	void parse_file(FILE *fp);
	bool all_vars_assigned();




	//void parse_file(std::string filename);
	void restart_solver();
	//int eliminate_unit();

public:
	CDCLSolver() : generator(random_generator()) {}
	void solve(FILE *fp, int time_limit);
	//void result_to_file(FILE *fp);
	void result_to_file(std::string filename);
	void set_model();
	void show_result();
};

#endif

