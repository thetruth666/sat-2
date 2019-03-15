#include "CDCLSolver.h"
using namespace std;

size_t mygetline(char** line, size_t *n, FILE *fp) {
	char *buf = *line;
	size_t c, i = 0;//i来记录字符串长度，c来存储字符
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

	sscanf(line, "p cnf %d %d", &nvar, &nclause);
	char *buffer = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
	char *last;

	size_t cnt = 0;
	while (!feof(fp)) {
		cnt += fread(buffer + cnt, 1, 40960, fp);
	}
	buffer[cnt] = 0;

	nAssigned = 0;
	antecedent_conflict = -1;
	pick_counter = 0;
	result = unknown;
	time_diff = 0;

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
			i++;
		}
		else {
			auto & v = vars[std::abs(literal) - 1];
			v.frequency++;
			v.polarity += (literal > 0 ? 1 : -1);
			v.var_clauses.reserve(64);
			v.var_clauses.push_back(make_pair(i, (literal > 0 ? 1 : -1)));
			clauses[i].literals.emplace_back(std::abs(literal), (literal > 0 ? 1 : -1));
		}
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
ss >> nvar;
ss >> nclause;
break;
}
}

nAssigned = 0;

antecedent_conflict = -1;
pick_counter = 0;

clauses.clear();
clauses.resize(nclause);
vars.clear();
vars.resize(nvar);

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
for (int i = 0; i < nvar; i++) {
vars[i] = { 0, -1, -1, 0, 0, 0 };   //初始化变元
}
for (int i = 0; i < nclause; i++) {
clauses[i].isSat = unknown;
}
}
*/
//根据给定文字给变元赋值

void CDCLSolver::restart_solver() {
	for (int i = 0; i < nvar; i++) {
		vars[i].value = 0;
		vars[i].frequency = vars[i].back_frequency;
	}
}
void CDCLSolver::assign(Literal _literal, int _antecedent, int _dlevel) {
	vars[_literal.var - 1].value = _literal.sign;
	vars[_literal.var - 1].antecedent = _antecedent;
	vars[_literal.var - 1].dlevel = _dlevel;

	vars[_literal.var - 1].back_frequency = vars[_literal.var - 1].frequency;
	vars[_literal.var - 1].frequency = -1;
	nAssigned++;
}

//返回变元赋值
void CDCLSolver::undo_assign(int var_idx) {
	vars[var_idx].value = 0;
	vars[var_idx].antecedent = -1;
	vars[var_idx].dlevel = -1;

	vars[var_idx].frequency = vars[var_idx].back_frequency;
	nAssigned--;
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
		vars[lit.var - 1].polarity += lit.sign;
		if (vars[lit.var - 1].frequency != -1) {
			vars[lit.var - 1].frequency++;
		}
		vars[lit.var - 1].back_frequency++;
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
	for (int i = 0; i < nvar; i++) {
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
	return nvar == nAssigned;
}

Literal CDCLSolver::pick_branch_var() {
	//产生随机数
	uniform_int_distribution<int> choose_branch(1, 10);
	uniform_int_distribution<int> choose_literal(0, nvar - 1);
	int random_value = choose_branch(generator);
	bool too_many_trys = false;
	int try_cnt = 0;
	do {
		//未赋值变量中根据频率选取
		if (random_value > 4 || nAssigned < nvar / 2 || too_many_trys) {
			pick_counter++;
			if (pick_counter == 20 * nvar) {
				for (int i = 0; i < vars.size(); i++) {
					vars[i].back_frequency /= 2;
					if (vars[i].frequency != -1) {
						vars[i].frequency /= 2;
					}
				}
				pick_counter = 0;
			}
			int tmp_var;
			int i = 0;
			//查找频率最大的未赋值项
			for (i = 0; i < nvar; i++) {
				if (vars[i].value == 0) {
					tmp_var = i;
					break;
				}
			}
			for (int j = i; j < nvar; j++) {
				if (vars[j].frequency > vars[tmp_var].frequency && vars[tmp_var].value == 0) {
					tmp_var = j;
				}
			}
			//给文字指派真值（根据极性）
			if (vars[tmp_var].polarity >= 0) {
				return
					Literal{ tmp_var + 1, 1 };
			}
			else return
				Literal{ tmp_var + 1, -1 };
		}
		else {
			//随机选取
			while (try_cnt < 10 * nvar) {
				int variable = choose_literal(generator);
				if (vars[variable].frequency != -1) {
					if (vars[variable].polarity >= 0) {
						return Literal{ variable + 1, 1 };
					}
					else return
						Literal{ variable + 1, -1 };
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
		for (int i = 0; i < nvar; i++) {
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

int CDCLSolver::unit_propagate(int _dlevel, Literal _lit, int antecedent) {
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
					return unsatisfied;
				}
				//出现unit
				if (unAssign_cnt == 1) {
					units.push({ cls_idx, unAssign_var });
				}
			}
		}
		//单没有单元子句存在时候
		bool unit_flag = false;
		while (!units.empty()) {
			ass_lit = units.front().lit;
			if (vars[ass_lit.var - 1].value == 0) {
				ass_antecedent = units.front().cls_idx;
				unit_flag = true;
				break;
			}
			else {
				units.pop();
			}
		}
		if (unit_flag == false) {
			break;
		}
	}
	if (all_vars_assigned())
		return satisfied;
	antecedent_conflict = -1;
	return unknown;
}

int CDCLSolver::begin_propagate() {
	int begin_result = satisfied;

	for (int i = 0; i < clauses.size(); i++) {
		if (clauses[i].isSat == true) {
			continue;
		}
		Literal unAssign_var;
		int unAssign_cnt = 0;
		int unsat_cnt = 0;
		for (int j = 0; j < clauses[i].literals.size(); j++) {
			Literal lit = clauses[i].literals[j];
			if (vars[lit.var - 1].value == lit.sign) {
				clauses[i].isSat = true;
				break;
			}
			else if (vars[lit.var - 1].value == 0) {
				unAssign_cnt++;
				unAssign_var = lit;
			}
			else
				unsat_cnt++;
		}
		if (unsat_cnt == clauses[i].literals.size()) {
			return unsatisfied;
		}
		else if (unAssign_cnt == 1) {
			begin_result = unit_propagate(0, unAssign_var, -1);
			if (begin_result == unsatisfied)
				return unsatisfied;
			else if (begin_result == unsatisfied)
				return satisfied;
		}
	}
	if (all_vars_assigned())
		return satisfied;
	return unknown;
}
void CDCLSolver::CDCL(int time_limit) {
	time_t start = clock();
	int dlevel = 0;
	Literal unit_literal;
	int var_propagate_result = unknown;
	/*
	if (eliminate_unit() == unsatisfied) {
	return unsatisfied;
	}
	*/
	//eliminate_unit 操作
	var_propagate_result = begin_propagate();
	time_t end = clock();
	if (difftime(end, start) > time_limit) {
		result = unknown;
		return;
	}
	if (var_propagate_result == unsatisfied) {
		result = unsatisfied;
		return;
	}
	else if (var_propagate_result == satisfied) {
		result = satisfied;
		return;
	}
	else {
		start = clock();
		do {
			//选择分支变量
			Literal pick_lit;
			int antecedent = -1;

			pick_lit = pick_branch_var();
			dlevel++;

			while (true) {
				if (pick_lit.var == -1) {
					break;
				}
				else {
					var_propagate_result = unit_propagate(dlevel, pick_lit, antecedent);
					if (var_propagate_result == unsatisfied) {
						if (dlevel == 0) {
							result = unsatisfied;
							return;
						}
						else {
							int back_level = conflict_analysis(dlevel);
							dlevel = back_jump(back_level);

							cls_lit tmp = search_unit();
							pick_lit = tmp.lit;
							antecedent = tmp.cls_idx;
						}
					}
					else if (var_propagate_result == satisfied) {
						result = satisfied;
						return;
					}
					else
						break;
				}
			}
			end = clock();
			if (difftime(end, start) > time_limit) {
				result = unknown;
				return;
			}
		} while (!all_vars_assigned());
	}
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
		for (int j = 0; j < clauses[i].literals.size(); j++) {
			Literal lit = clauses[i].literals[j];
			if (vars[lit.var - 1].value == lit.sign) {
				clauses[i].isSat = true;
				break;
			}
			else if (vars[lit.var - 1].value == 0) {
				unAssign_cnt++;
				unAssign_var = lit;
			}
		}
		if (unAssign_cnt == 1) {
			return { i, unAssign_var };
		}
	}
	return { -1, { -1, -1 } };
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
nclause--;
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
nclause--;
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
	std::ofstream file(filename);
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
		for (int i = 0; i < nvar; i++) {
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
	//show_result();
}

