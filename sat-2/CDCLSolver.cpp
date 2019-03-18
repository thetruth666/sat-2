#include "CDCLSolver.h"
#include <cassert>
using namespace std;

size_t mygetline(char** line, size_t *n, FILE *fp) {
	char *buf = *line;
	char c;
	size_t i = 0;//i����¼�ַ������ȣ�c���洢�ַ�
	if (buf == NULL || *n == 0) {
		*line = (char *)malloc(10);
		buf = *line;
		*n = 10;
	}
	//bufΪ��nΪ0ʱ��̬Ϊ�ڷ���ռ�
	while ((c = fgetc(fp)) != '\n') {
		if (c == EOF)
			return -1;
		//��2���ռ��\n��\0
		if (i < *n - 2) {
			*(buf + i++) = c;
		}
		else {
			*n = *n + 10;
			buf = (char *)realloc(buf, *n);//�ռ䲻��ʱ�����½��з���
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
			//�����Ӿ䳤�ȶԱ������ߵ�Ӱ��
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
	//��ʼ�����ȼ�
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
//��ȡ�Ӿ���Ϣ,��ʼ���Ӿ���,��Ԫ��
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
vars[i] = { 0, -1, -1, 0, 0, 0 };   //��ʼ����Ԫ
}
for (int i = 0; i < clauses.size(); i++) {
clauses[i].isSat = unknown;
}
}
*/
//���ݸ������ָ���Ԫ��ֵ
//�����㷨�Ƿ���Ҫ����ѧϰ�Ӿ���Ϣ
void CDCLSolver::restart_solver() {
	//������Ԫ
	nAssigned = 0;
	for (int i = 0; i < vars.size(); i++) {
		vars[i].value = 0;
		vars[i].back_priority = vars[i].priority = N * (vars[i].pos * vars[i].neg) + M * vars[i].length_score;
		vars[i].antecedent = -1;
		vars[i].dlevel = 0;
	}
	//�����Ӿ伯
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

//���ر�Ԫ��ֵ
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
int unsat_cnt = 0;                     //ͳ�Ƶ��Ӿ���δ�����������
int unAssign_cnt = 0;                  //ͳ�Ƶ��Ӿ���δ��ֵ��������

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
		//�ҵ�UIP
		if (this_level_cnt == 1) {
			break;
		}
		learn_clause = resolve(learn_clause, tmp_idx);
	}
	clauses.push_back(learn_clause);

	//����������Ϣ
	for (int i = 0; i < learn_clause.literals.size(); i++) {
		Literal lit = learn_clause.literals[i];
		if (lit.sign > 0)
			vars[lit.var - 1].pos++;
		else
			vars[lit.var - 1].neg++;
		//ѧϰ�Ӿ�һ��ϳ��������ܹ��ֽ����������ȼ�,
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
//�����ȥ��
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
		//ɾ����UIP
		if (learn_clause.literals[i].var - 1 == var_idx) {
			learn_clause.literals.erase(learn_clause.literals.begin() + i);
			i--;
		}
	}
	//ɾ���ظ�
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
				//���ܴ˱�Ԫ��ֵ���Ӿ����������Ӱ��
				if (vars[i].var_clauses[j].second == vars[i].value) {
					int cls_idx = vars[i].var_clauses[j].first;
					for (int n = 0; n < clauses[cls_idx].literals.size(); n++) {
						Literal lit = clauses[cls_idx].literals[n];
						//һ���Ӿ��л���Ϊ�渳ֵ�Ҿ�����ǰ���򲻸ı��Ӿ�Ϊ�����ʵ
						if (vars[lit.var - 1].dlevel <= back_dlevel && vars[lit.var - 1].value == lit.sign) {
							sat_falg = true;
							break;
						}
					}
					//�Ӿ���û�о�����ǰ��Ϊ�渳ֵ��
					clauses[cls_idx].isSat = sat_falg;
				}
			}
			undo_assign(i);    //���� ���ݲ�����߲�֮��ľ��߲�
		}
	}
	return back_dlevel;
}

bool CDCLSolver::all_vars_assigned() {
	return vars.size() == nAssigned;
}

Literal CDCLSolver::pick_branch_var() {
	//���������
	assert(!all_vars_assigned());
	uniform_int_distribution<int> choose_branch(1, 10);
	uniform_int_distribution<int> choose_literal(0, vars.size() - 1);
	int random_value = choose_branch(generator);
	bool too_many_trys = false;
	int try_cnt = 0;
	do {
		//δ��ֵ�����и���Ƶ��ѡȡ
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
			//����Ƶ������δ��ֵ��
			for (int i = 0; i < vars.size(); i++) {
				if (vars[i].value == 0 && vars[i].priority > vars[tmp_var].priority) {
					tmp_var = i;
				}
			}
			//������ָ����ֵ�����ݼ��ԣ�
			if (vars[tmp_var].pos >= vars[tmp_var].neg) {
				return Literal{ tmp_var + 1, 1 };
			}
			else
				return Literal{ tmp_var + 1, -1 };
		}
		else {
			//���ѡȡ
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
		//�����鿴�˴θ�ֵ�Թ����Ӿ�Ӱ��Ľ��
		for (int i = 0; i < vars[ass_lit.var - 1].var_clauses.size(); i++) {
			int cls_idx = vars[ass_lit.var - 1].var_clauses[i].first;
			//�Ӿ��ڱ��θ�ֵ���Ѿ�����
			if (clauses[cls_idx].isSat == true) {
				continue;
			}
			//�Ӿ��ڱ��θ�ֵ����
			if (ass_lit.sign == vars[ass_lit.var - 1].var_clauses[i].second) {
				clauses[cls_idx].isSat = true;
			}
			//�Ӿ�������δ����
			//����Ӿ���unit����unsat
			else {
				Literal unAssign_var;
				int unAssign_cnt = 0;
				int unsat_cnt = 0;
				for (int j = 0; j < clauses[cls_idx].literals.size(); j++) {
					Literal lit = clauses[cls_idx].literals[j];
					//����δ��ֵ
					if (vars[lit.var - 1].value == 0) {
						unAssign_cnt++;
						unAssign_var = lit;
					}
					else if (vars[lit.var - 1].value == -lit.sign) {
						unsat_cnt++;
					}
				}
				//����ì���Ӿ�
				if (unsat_cnt == clauses[cls_idx].literals.size()) {
					antecedent_conflict = cls_idx;
					return false;
				}
				//����unit
				if (unAssign_cnt == 1) {
					units.push({ cls_idx, unAssign_var });
				}
			}
		}
		//��û�е�Ԫ�Ӿ����ʱ��
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
			else { //��ʾ�˵����������ڱ�Ĵ����и�ֵ��
				units.pop();
			}
		}
		//�Ѿ�û�е��Ӿ���
		if (unit_flag == false) {
			break;
		}
	}
	antecedent_conflict = -1;
	return true;
}

//��ͻ�������µ��Ӿ���������unit-propagation���������ֻҪ�鿴ԭ���Ӿ��������
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
	//eliminate_unit ����
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
		//ѡ���֧����
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
				//����ì���ڶ���ֱ�����������
				if (dlevel == 0) {
					result = unsatisfied;
					return;
				}
				//����ì�ܲ��ڶ�����г�ͻѧϰ
				else {
					int back_level = conflict_analysis(dlevel);
					dlevel = back_jump(back_level);
					conflict_flag = true;
				}
			}
			//δ����ì��
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
		//���ݺ��Ƿ��Դ��ڳ�ͻ��
		if (unAssign_cnt == 1) {
			return { i, unAssign_var };
		}
	}
	return { -1, { -1, -1 } };
}
void CDCLSolver::set_model() {
	int choose = 0;
	cout << "����ѡȡ��֧ģʽ :(0:��� 1:��Ȩ 2:���+��Ȩ(�������) 3:���+��Ȩ(��Ȩ����) 4:Ĭ��" << endl;
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
//ԭcnf���Ƿ��е�Ԫ�Ӿ䣨ɾ����Ԫ�Ӿ䣿��
for (int i = 0; i < clauses.size(); i++) {
if (clauses[i].literals.size() == 1) {
Literal lit = clauses[i].literals[0];
//��Ϊȷ����ֵ
vars[lit.var - 1].value = lit.sign;
vars[lit.var - 1].antecedent = -1;
vars[lit.var - 1].frequency = -1;
vars[lit.var - 1].dlevel = 0;

stack<int> units;
do {
for (int i = 0; i < vars[lit.var - 1].var_clauses.size(); i++) {
int cls_idx = vars[lit.var - 1].var_clauses[i].first;
//������ֱ��ɾ��
if (lit.sign == vars[lit.var - 1].var_clauses[i].second) {
clauses[cls_idx].isSat = true;
//clauses.erase(clauses.begin() + cls_idx);
clauses.size()--;
}
//����ȥͳ�Ʋ�����unit_cls
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
	std::ofstream file("F:\\��������\\sat-2\\sat-2\\output\\" + filename);
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

