#include "CDCLSolver.h"
#include <cassert>
using namespace std;

size_t mygetline(char** line, size_t *n, FILE *fp) {
	char *buf = *line;
	char c;
	size_t i = 0;//i来记录字符串长度，c来存储字符
	if (buf == NULL || *n == 0) {
		*line = (char *)malloc(10);
		buf = *line;
		*n = 10;
	}
	//buf为或n为0时动态为期分配空间
	while ((c = fgetc(fp)) != '\n') {
		if (c == EOF)
			return -1;
		//留2个空间给\n和\0
		if (i < *n - 2) {
			*(buf + i++) = c;
		}
		else {
			*n = *n + 10;
			buf = (char *)realloc(buf, *n);//空间不足时，重新进行分配
			*(buf + i++) = c;
		}
	}
	*(buf + i++) = '\n';
	*(buf + i) = '\0';
	return i;
}

void CDCLSolver::parse_file(FILE *fp) {
	size_t len = 200;
	char *line = (char*)malloc(sizeof(char) * len);

	do {
		mygetline(&line, &len, fp);
	} while (line[0] == 'c');
	int nvar, nclause;
	sscanf(line, "p cnf %d %d", &nvar, &nclause);
	char *buffer = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
	char *last;

	size_t cnt = 0;
	while (!feof(fp)) {
		cnt += fread(buffer + cnt, 1, 40960, fp);
	}
	buffer[cnt] = 0;

	clauses.clear();
	clauses.resize(nclause);
	vars.clear();
	vars.resize(nvar);

	int literal;
	for (int i = 0; i < nclause;) {

		last = buffer;
		while (*buffer != ' ' && *buffer != '\n') buffer++;
		*buffer++ = 0;
		literal = atoi(last);
		if (literal == 0) {
			//加上子句长度对变量决策的影响
			int tmp_size = clauses[i].literals.size();
			for (int j = 0; j < tmp_size; j++) {
				vars[clauses[i].literals[j].var - 1].length_score += 20 - tmp_size;
			}
			i++;
		}
		else {
			auto & v = vars[std::abs(literal) - 1];
			literal > 0 ? v.pos++ : v.neg++;
			v.var_clauses.reserve(64);
			v.var_clauses.push_back(make_pair(i, (literal > 0 ? 1 : -1)));
			clauses[i].literals.emplace_back(std::abs(literal), (literal > 0 ? 1 : -1));
		}
	}
	//初始化优先级
	for (int i = 0; i < nvar; i++) {
		vars[i].priority = N * (vars[i].pos * vars[i].neg) + M * vars[i].length_score;
	}
}
/*
void CDCLSolver::parse_file(string filename) {
ifstream file(filename, ios::in | ios::app);
if (!file)
return;
string line;
stringstream ss;
while (true) {
getline(file, line);
if (line[0] == 'c') {
line.clear();
}
else if (line.find("p cnf") != string::npos) {
line.erase(0, 6);
ss.str(line);
ss >> vars.size();
ss >> clauses.size();
break;
}
}

nAssigned = 0;

antecedent_conflict = -1;
pick_counter = 0;

clauses.clear();
clauses.resize(clauses.size());
vars.clear();
vars.resize(vars.size());

int literal;
int i = 0;
while (getline(file, line)) {
ss.clear();
ss.str(line);
//读取子句信息,初始化子句组,变元组
while (true) {
ss >> literal;
if (!literal)
break;
vars[std::abs(literal) - 1].frequency++;
vars[std::abs(literal) - 1].polarity += (literal > 0 ? 1 : -1);
vars[std::abs(literal) - 1].var_clauses.push_back(make_pair(i, (literal > 0 ? 1 : -1)));
clauses[i].literals.push_back(Literal{ std::abs(literal), (literal > 0 ? 1 : -1) });
}
i++;
}
}
*/

/*
void CDCLSolver::reset_solver() {
for (int i = 0; i < vars.size(); i++) {
vars[i] = { 0, -1, -1, 0, 0, 0 };   //初始化变元
}
for (int i = 0; i < clauses.size(); i++) {
clauses[i].isSat = unknown;
}
}
*/
//根据给定文字给变元赋值
//重启算法是否需要重启学习子句信息
void CDCLSolver::restart_solver() {
	//重启变元
	nAssigned = 0;
	for (int i = 0; i < vars.size(); i++) {
		vars[i].value = 0;
		vars[i].back_priority = vars[i].priority = N * (vars[i].pos * vars[i].neg) + M * vars[i].length_score;
		vars[i].antecedent = -1;
		vars[i].dlevel = 0;
	}
	//重启子句集
	for (int j = 0; j < clauses.size(); j++) {
		clauses[j].isSat = false;
	}
}

void CDCLSolver::assign(Literal _literal, int _antecedent, int _dlevel) {
	assert(vars[_literal.var - 1].value == 0);
	//cout << "assign : " << _literal.var * _literal.sign << "." << _antecedent << "." << _dlevel << endl;
	vars[_literal.var - 1].value = _literal.sign;
	vars[_literal.var - 1].antecedent = _antecedent;
	vars[_literal.var - 1].dlevel = _dlevel;

	vars[_literal.var - 1].back_priority = vars[_literal.var - 1].priority;
	vars[_literal.var - 1].priority = -1;
	nAssigned++;
	//cout << nAssigned << endl;
}

//返回变元赋值
void CDCLSolver::undo_assign(int var_idx) {
	assert(vars[var_idx].value != 0);
	vars[var_idx].value = 0;
	vars[var_idx].antecedent = -1;
	vars[var_idx].dlevel = -1;
	//cout << "undoassign : " << var_idx + 1 << endl;
	vars[var_idx].priority = vars[var_idx].back_priority;
	nAssigned--;
	//cout << nAssigned << endl;
}

/*
int CDCLSolver::clause_status(Clause _clause) {
if (_clause.status != 0) {
int unsat_cnt = 0;

for (int i = 0; i < _clause.literals.size(); i++) {
int var = _clause.literals.at[i].var;
int sign = _clause.literals.at[i].sign;
if (vars[var].value == sign) {
_clause.status = satisfied;
return 0;
}
else {
unsat_cnt++;
}
}
if (unsat_cnt = 1) {
return true;
}
else
return false;
}
}
*/

/*
int CDCLSolver::unit_propagate(int dlevel) {
bool sat_flag = false;
bool unit_found = false;
Literal unAssign_literal;

do {
unit_found = false;
for (int i = 0; i < clauses.size() && !unit_found; i++) {
int unsat_cnt = 0;                     //统计单子句中未满足的文字数
int unAssign_cnt = 0;                  //统计单子句中未赋值的文字数

*/
/*
if (clauses[i].isSat == satisfied) {
break;
}
else {

sat_flag = false;
for (int j = 0; j < clauses[i].literals.size(); j++) {
int var_idx = clauses[i].literals[j].var - 1;
int sign = clauses[i].literals[j].sign;
if (vars[var_idx].value == sign) {
clauses[i].isSat = satisfied;
sat_flag = true;
break;
}
else if (vars[var_idx].value == -sign) {
unsat_cnt++;
}
else {
unAssign_cnt++;
unAssign_literal = clauses[i].literals[j];
}
}
*/
/*
if (sat_flag == true) {
continue;
}
if (unAssign_cnt == 1) {
assign(unAssign_literal, i, dlevel);
unit_found = true;
break;
}

if (unsat_cnt == clauses[i].literals.size()) {
antecedent_conflict = i;
return unsatisfied;
}
//}
}
} while (unit_found);

antecedent_conflict = -1;
return unknown;
}
*/
int CDCLSolver::conflict_analysis(int dlevel) {
	conflict_cnt++;
	Clause learn_clause = clauses[antecedent_conflict];
	int this_level_cnt = 0;
	int back_dlevel = -1;
	int tmp_idx = 0;

	while (true) {
		this_level_cnt = 0;
		for (int i = 0; i < learn_clause.literals.size(); i++) {
			int var_idx = learn_clause.literals[i].var - 1;
			if (vars[var_idx].dlevel == dlevel) {
				this_level_cnt++;

				if (vars[var_idx].antecedent != -1)
					tmp_idx = var_idx;
			}
		}
		//找到UIP
		if (this_level_cnt == 1) {
			break;
		}
		learn_clause = resolve(learn_clause, tmp_idx);
	}
	clauses.push_back(learn_clause);

	//更新文字信息
	for (int i = 0; i < learn_clause.literals.size(); i++) {
		Literal lit = learn_clause.literals[i];
		if (lit.sign > 0)
			vars[lit.var - 1].pos++;
		else
			vars[lit.var - 1].neg++;
		//学习子句一般较长，当不能过分降低它的优先级,
		vars[lit.var - 1].length_score += learn_base - learn_clause.literals.size();
		if (vars[lit.var - 1].value == 0) {
			vars[lit.var - 1].priority = N *(vars[lit.var - 1].pos * vars[lit.var - 1].neg) + M * vars[lit.var - 1].length_score;
		}
		vars[lit.var - 1].back_priority = N *(vars[lit.var - 1].pos * vars[lit.var - 1].neg) + M * vars[lit.var - 1].length_score;
		vars[lit.var - 1].var_clauses.push_back(make_pair((clauses.size() - 1), lit.sign));
		if (vars[lit.var - 1].dlevel > back_dlevel && vars[lit.var - 1].dlevel != dlevel) {
			back_dlevel = vars[lit.var - 1].dlevel;
		}
	}
	return back_dlevel;
}
//排序后去重
void delete_sort_dup(Clause &container) {
	for (int i = 0; i < container.literals.size() - 1; i++) {
		if (container.literals[i] == container.literals[i + 1]) {
			container.literals.erase(container.literals.begin() + i);
		}
	}
}
Clause &CDCLSolver::resolve(Clause &learn_clause, int var_idx) {
	Clause input_clause = clauses[vars[var_idx].antecedent];
	learn_clause += input_clause;
	for (int i = 0; i < learn_clause.literals.size(); i++) {
		//删除非UIP
		if (learn_clause.literals[i].var - 1 == var_idx) {
			learn_clause.literals.erase(learn_clause.literals.begin() + i);
			i--;
		}
	}
	//删除重复
	sort(learn_clause.literals.begin(), learn_clause.literals.end(), [](const Literal &a, const Literal &b)->bool {return a.var < b.var; });
	delete_sort_dup(learn_clause);
	//learn_clause.literals.erase(unique(learn_clause.literals.begin(), learn_clause.literals.end()), learn_clause.literals.end());
	return learn_clause;
}


int CDCLSolver::back_jump(int back_dlevel) {
	for (int i = 0; i < vars.size(); i++) {
		if (vars[i].dlevel > back_dlevel) {
			for (int j = 0; j < vars[i].var_clauses.size(); j++) {
				bool sat_falg = false;
				//可能此变元赋值对子句可满足性有影响
				if (vars[i].var_clauses[j].second == vars[i].value) {
					int cls_idx = vars[i].var_clauses[j].first;
					for (int n = 0; n < clauses[cls_idx].literals.size(); n++) {
						Literal lit = clauses[cls_idx].literals[n];
						//一旦子句中还有为真赋值且决策在前，则不改变子句为真的事实
						if (vars[lit.var - 1].dlevel <= back_dlevel && vars[lit.var - 1].value == lit.sign) {
							sat_falg = true;
							break;
						}
					}
					//子句中没有决策在前的为真赋值了
					clauses[cls_idx].isSat = sat_falg;
				}
			}
			undo_assign(i);    //回溯 回溯层与决策层之间的决策层
		}
	}
	return back_dlevel;
}

bool CDCLSolver::all_vars_assigned() {
	return vars.size() == nAssigned;
}

Literal CDCLSolver::pick_branch_var() {
	//产生随机数
	assert(!all_vars_assigned());
	uniform_int_distribution<int> choose_branch(1, 10);
	uniform_int_distribution<int> choose_literal(0, vars.size() - 1);
	int random_value = choose_branch(generator);
	bool too_many_trys = false;
	int try_cnt = 0;
	do {
		//未赋值变量中根据频率选取
		if (!isRand && (random_value > probability || nAssigned < vars.size() / 2 || too_many_trys)) {
			pick_counter++;
			if (pick_counter == 20 * vars.size()) {
				for (int i = 0; i < vars.size(); i++) {
					vars[i].back_priority /= 2;
					if (vars[i].value != 0) {
						vars[i].priority /= 2;
					}
				}
				pick_counter = 0;
			}
			int tmp_var = 0;
			//查找频率最大的未赋值项
			for (int i = 0; i < vars.size(); i++) {
				if (vars[i].value == 0 && vars[i].priority > vars[tmp_var].priority) {
					tmp_var = i;
				}
			}
			//给文字指派真值（根据极性）
			if (vars[tmp_var].pos >= vars[tmp_var].neg) {
				return Literal{ tmp_var + 1, 1 };
			}
			else
				return Literal{ tmp_var + 1, -1 };
		}
		else {
			//随机选取
			while (try_cnt < 10 * vars.size()) {
				int variable = choose_literal(generator);
				if (vars[variable].value == 0) {
					if (vars[variable].pos >= vars[variable].neg) {
						return Literal{ variable + 1, 1 };
					}
					else
						return Literal{ variable + 1, -1 };
				}
				try_cnt++;
			}
			too_many_trys = true;
		}
	} while (too_many_trys);
}

void CDCLSolver::show_result() {
	if (result == satisfied) {
		cout << "SAT" << endl;
		for (int i = 0; i < vars.size(); i++) {
			cout << vars[i].value * (i + 1) << ' ';
		}
		cout << endl;
		cout << time_diff << "ms" << endl;
		return;
	}
	else if (result == unknown) {
		cout << "UNKNOWN";
		return;
	}
	else {
		cout << "UNSAT";
		cout << time_diff << "ms" << endl;
		return;
	}
}

bool CDCLSolver::unit_propagate(int _dlevel, Literal _lit, int antecedent) {
	queue<cls_lit> units;

	Literal ass_lit = _lit;
	int ass_antecedent = antecedent;

	while (true) {
		assign(ass_lit, ass_antecedent, _dlevel);
		//遍历查看此次赋值对关联子句影响的结果
		for (int i = 0; i < vars[ass_lit.var - 1].var_clauses.size(); i++) {
			int cls_idx = vars[ass_lit.var - 1].var_clauses[i].first;
			//子句在本次赋值外已经满足
			if (clauses[cls_idx].isSat == true) {
				continue;
			}
			//子句在本次赋值满足
			if (ass_lit.sign == vars[ass_lit.var - 1].var_clauses[i].second) {
				clauses[cls_idx].isSat = true;
			}
			//子句至今尚未满足
			//检查子句是unit还是unsat
			else {
				Literal unAssign_var;
				int unAssign_cnt = 0;
				int unsat_cnt = 0;
				for (int j = 0; j < clauses[cls_idx].literals.size(); j++) {
					Literal lit = clauses[cls_idx].literals[j];
					//文字未赋值
					if (vars[lit.var - 1].value == 0) {
						unAssign_cnt++;
						unAssign_var = lit;
					}
					else if (vars[lit.var - 1].value == -lit.sign) {
						unsat_cnt++;
					}
				}
				//出现矛盾子句
				if (unsat_cnt == clauses[cls_idx].literals.size()) {
					antecedent_conflict = cls_idx;
					return false;
				}
				//出现unit
				if (unAssign_cnt == 1) {
					units.push({ cls_idx, unAssign_var });
				}
			}
		}
		//当没有单元子句存在时候
		bool unit_flag = false;
		if (all_vars_assigned()) {
			return true;
		}
		while (!units.empty()) {
			ass_lit = units.front().lit;
			if (vars[ass_lit.var - 1].value == 0) {
				ass_antecedent = units.front().cls_idx;
				unit_flag = true;
				break;
			}
			else { //表示此单变量可能在别的传播中赋值了
				units.pop();
			}
		}
		//已经没有单子句了
		if (unit_flag == false) {
			break;
		}
	}
	antecedent_conflict = -1;
	return true;
}

//冲突，产生新单子句的情况均在unit-propagation解决，所以只要查看原单子句情况即可
bool CDCLSolver::begin_propagate() {
	bool begin_result = true;
	Literal unit_var;;
	queue<Literal> unit_queue;
	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].literals.size() == 1) {
			unit_queue.push(clauses[i].literals[0]);
		}
	}
	while (!unit_queue.empty()) {
		unit_var = unit_queue.front();
		unit_queue.pop();
		if (vars[unit_var.var - 1].value != 0) continue;
		begin_result = unit_propagate(0, unit_var, -1);
		if (begin_result == false) {
			return false;
		}
	}
	return true;
}

void CDCLSolver::CDCL(int time_limit) {
	time_t start = clock();
	int dlevel = 0;
	Literal unit_literal;
	int unit_result = false;
	/*
	if (eliminate_unit() == unsatisfied) {
	return unsatisfied;
	}
	*/
	//eliminate_unit 操作
	unit_result = begin_propagate();
	time_t end = clock();
	if (difftime(end, start) > time_limit) {
		result = unknown;
		return;
	}
	if (unit_result == false) {
		result = unsatisfied;
		return;
	}
	else {
		if (all_vars_assigned()) {
			result = satisfied;
			return;
		}
	}

	start = clock();
	while (true) {
		assert(nAssigned <= vars.size());
		//选择分支变量
		bool conflict_flag = false;
		cls_lit ass_cls_lit = { -1, { -1, -1 } };
		Literal pick_lit;
		int antecedent = -1;
		pick_lit = pick_branch_var();
		//cout << "branch" << pick_lit.var * pick_lit.sign << endl;
		dlevel++;

		while (true) {
			unit_result = unit_propagate(dlevel, pick_lit, antecedent);
			if (!unit_result) {
				//出现矛盾在顶层直接输出不满足
				if (dlevel == 0) {
					result = unsatisfied;
					return;
				}
				//出现矛盾不在顶层进行冲突学习
				else {
					int back_level = conflict_analysis(dlevel);
					dlevel = back_jump(back_level);
					conflict_flag = true;
				}
			}
			//未出现矛盾
			else {
				if (all_vars_assigned()) {
					result = satisfied;
					return;
				}
			}
			if (conflict_flag) {
				ass_cls_lit = search_unit();
				antecedent = ass_cls_lit.cls_idx;
				pick_lit = ass_cls_lit.lit;
				if (antecedent == -1) {
					break;
				}
			}
			else {
				break;
			}
		}
		end = clock();
		if (difftime(end, start) > time_limit) {
			result = unknown;
			return;
		}
		if (conflict_cnt > 1000) {
			restart_solver();
			conflict_cnt = 0;
			dlevel = 0;
			//cout << clauses.size() << endl;
		}
		//cout << "conflict_cnt : " << conflict_cnt << endl;
	};
	result = satisfied;
	return;
}

cls_lit CDCLSolver::search_unit() {
	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].isSat == true) {
			continue;
		}
		Literal unAssign_var;
		int unAssign_cnt = 0;
		for (int j = clauses[i].literals.size() - 1; j > -1; j--) {
			Literal lit = clauses[i].literals[j];
			/*if (vars[lit.var - 1].value == lit.sign) {
				clauses[i].isSat = true;
				break;
			}
			else */if (vars[lit.var - 1].value == 0) {
				unAssign_cnt++;
				unAssign_var = lit;
			}
		}
		//回溯后是否仍存在冲突？
		if (unAssign_cnt == 1) {
			return { i, unAssign_var };
		}
	}
	return { -1, { -1, -1 } };
}
void CDCLSolver::set_model() {
	int choose = 0;
	cout << "采用选取分支模式 :(0:随机 1:加权 2:随机+加权(随机优先) 3:随机+加权(加权优先) 4:默认" << endl;
	cin >> choose;
	if (choose == 0) {
		isRand = true;
	}
	else if (choose == 1) {
		probability = 0;
	}
	else if (choose == 2) {
		probability = 8;
	}
	else if (choose == 3) {
		probability = 2;
	}
}
/*
int CDCLSolver::eliminate_unit() {
//原cnf中是否有单元子句（删除单元子句？）
for (int i = 0; i < clauses.size(); i++) {
if (clauses[i].literals.size() == 1) {
Literal lit = clauses[i].literals[0];
//此为确定赋值
vars[lit.var - 1].value = lit.sign;
vars[lit.var - 1].antecedent = -1;
vars[lit.var - 1].frequency = -1;
vars[lit.var - 1].dlevel = 0;

stack<int> units;
do {
for (int i = 0; i < vars[lit.var - 1].var_clauses.size(); i++) {
int cls_idx = vars[lit.var - 1].var_clauses[i].first;
//满足则直接删除
if (lit.sign == vars[lit.var - 1].var_clauses[i].second) {
clauses[cls_idx].isSat = true;
//clauses.erase(clauses.begin() + cls_idx);
clauses.size()--;
}
//否则去统计并储存unit_cls
else {
Literal unAssign_var;
for (int j = 0; j < clauses[clause_idx].literals.size(); j++) {
int unAssign_cnt = 0;

Literal tmp_lit = clauses[clause_idx].literals[j];
if (vars[tmp_lit.var - 1].value == 0) {
unAssign_cnt++;
unAssign_var = clauses[clause_idx].literals[j];
unit_var = unAssign_var.var;
}
else {
antecedent_conflict = vars[dlevel_var].var_clauses[i].first;
return unsatisfied;
}
if (unAssign_cnt == 1) {
assign(unAssign_var, clause_idx, 0);
clauses.erase(clauses.begin() + clause_idx);
clauses.size()--;
units.push(unit_var);
}
}
}
}
if (!units.empty()) {
dlevel_var = units.top();
units.pop();
}
else
break;
} while (true);
}
}

if (all_vars_assigned())
return satisfied;
return unknown;
}
*/
void CDCLSolver::result_to_file(string filename) {
	std::ofstream file("F:\\数独终盘\\sat-2\\sat-2\\output\\" + filename);
	if (!file)
		exit(0);
	if (result == unsatisfied) {
		file << "s 0" << endl;
		file << "t " << time_diff << "ms" << endl;
	}
	else if (result == unknown) {
		file << "s -1" << endl;
	}
	else {
		file << "s 1" << endl;
		file << "v ";
		for (int i = 0; i < vars.size(); i++) {
			file << vars[i].value * (i + 1) << ' ';
		}
		file << '0' << endl;
		file << "t " << time_diff << "ms" << endl;
	}
}
void CDCLSolver::solve(FILE *fp, int timelimit) {
	parse_file(fp);
	time_t start, end;
	start = clock();
	CDCL(timelimit);
	end = clock();
	time_diff = difftime(end, start);
	cout << "time_diff : " << time_diff << endl;
	cout << "conflict_cnt : " << conflict_cnt << endl;
	//show_result();
}

