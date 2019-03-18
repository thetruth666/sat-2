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

	int antecedent_conflict = -1;
	int nAssigned = 0;
	int pick_counter = 0;
	int result = unknown;

	int probability = 4;
	int conflict_cnt = 0;


	int M = 2;             //子句长参数
	int N = 7;             //极性参数
	bool isRand = false;
	int learn_base = 100;

	double time_diff = 0.0;

	std::random_device random_generator;
	std::mt19937 generator;

	void assign(Literal _literal, int antecedent, int dlevel);
	void undo_assign(int _var);
	//BCP
	void CDCL(int time_limit);
	cls_lit search_unit();
	bool begin_propagate();
	bool unit_propagate(int _dlevel, Literal lit, int antecedent);

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

