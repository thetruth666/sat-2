#include "common.h"
#include "CDCLSolver.h"
#include "Sudoku.h"
#include "fstream"

using namespace std;
bool print_cnf(string filename);
void print_option();
void choose_option();
void print_sat_option();
void choose_sat_option();
void print_sudoku_option();
void choose_suduku_option();

void check_solve(std::string cnf_filename, std::string solved_filename) {
	vector<vector<int>> check_clauses;
	vector<int> check_literals;
	vector<int> check_vars;
	int var_cnt, cls_cnt;
	std::ifstream file_cnf(cnf_filename);
	std::ifstream file_solved(solved_filename);
	string line;
	if (!file_cnf || !file_solved)
		return;
	stringstream ss;
	while (true) {
		getline(file_cnf, line);
		if (line[0] == 'c') {
			line.clear();
		}
		else if (line.find("p cnf") != string::npos) {
			line.erase(0, 6);
			ss.str(line);
			ss >> var_cnt;
			ss >> cls_cnt;
			break;
		}
	}

	int literal;
	for (int i = 0; i < cls_cnt; i++) {
		getline(file_cnf, line);
		ss.clear();
		ss.str(line);
		//��ȡ�Ӿ���Ϣ,��ʼ���Ӿ���,��Ԫ��
		while (true) {
			ss >> literal;
			if (!literal)
				break;
			else {
				check_literals.push_back(literal);
			}
		}
		check_clauses.push_back(check_literals);
		check_literals.clear();
		line.clear();
	}
	ss.clear();
	ss << file_solved.rdbuf();
	ss.seekg(0, ios::beg);
	ss.seekg(6, ios::cur);

	while (true) {
		ss >> literal;
		if (literal == 0)
			break;
		check_vars.push_back(literal);
	}

	bool flag = false;
	for (int i = 0; i < cls_cnt; i++) {
		flag = false;
		//cout << i <<endl;
		for (int j = 0; j < check_clauses[i].size(); j++) {
			int literal = check_clauses[i][j];
			if (check_vars[abs(literal) - 1] == literal) {
				//cout << literal << " with " << i << ": " << check_vars[abs(literal) - 1] << endl;
				flag = true;
				break;
			}
		}
		if (!flag) {
			cout << "wrong: line " << i << ":";
			for (int n = 0; n < check_clauses[i].size(); n++) {
				cout << check_clauses[i][n] << ' ';
			}
			return;
		}
	}
	cout << "correct";
}
void print_option() {
	system("cls");
	cout << "---------- SAT SOLVER AND SUDOKU ----------" << endl;
	cout << "1 : SAT �����." << endl;
	cout << "2 : SUDOKU ������." << endl;
	cout << "0 : �˳�." << endl << endl;
}
void choose_option() {
	cout << "������ѡ�� : ";
	int i;
	do {
		cin >> i;
		if (i == 1) {
			print_sat_option();
			choose_sat_option();
		}
		else if (i == 2) {
			print_sudoku_option();
			choose_suduku_option();
		}
		else if (i == 0) {
			return;
		}
		print_option();
	} while (i != 0);
	return;
}
void print_sat_option() {
	system("cls");
	cout << "---------- SAT SOLVER ----------" << endl;
	cout << "1 : ���������������." << endl;
	cout << "2 : �����ļ����ļ���ʽΪ(xxx.cnf)." << endl;
	cout << "3 : ��ӡcnf�ļ�." << endl;
	cout << "4 : ����ʱ������,Ĭ��Ϊ100000ms." << endl;
	cout << "5 : ��ӡ������." << endl;
	cout << "6 : �����ļ����ϣ�.txt��." << endl;
	cout << "7 : �������ȷ��." << endl;
	cout << "0 : �ر�." << endl;
}
void choose_sat_option() {
	int i;
start:
	cout << "������ѡ�� : ";
	cin >> i;
	FILE *fp_in = nullptr;
	string filename;
	string outname;
	string set_filename;
	int time_limit = 1000000;
	bool solved_flag = false;

	if (i == 6) {
		cout << "�����ļ��� : ";
		cin >> set_filename;
		std::ifstream set_file(set_filename);
		while (true) {
			getline(set_file, filename);
			if (!set_file) {
				break;
			}
			fp_in = fopen(filename.c_str(), "r");
			CDCLSolver solver;
			solver.solve(fp_in, time_limit);
			outname = "solved-" + filename;
			solver.result_to_file(outname);
			fclose(fp_in);
		}
		cout << "solved" << endl;
		goto start;
	}
	CDCLSolver solver;
	do {
		solved_flag = false;
		switch (i) {
		case 1:
			if (!fp_in) {
				cout << "���ȴ��ļ�." << endl;
				break;
			}
			solver.solve(fp_in, time_limit);
			solved_flag = true;
			fclose(fp_in);
			outname = "solved-" + filename + ".ies";
			solver.result_to_file(outname);
			cout << "������." << endl;
			break;
		case 2:
			cout << "�������ļ����� : " << endl;
			cin >> filename;
			fp_in = fopen(filename.c_str(), "r");
			if (!fp_in) {
				cout << "�����ļ�ʧ��." << endl;
				break;
			}
			cout << "�����ļ� " + filename + " �ɹ�." << endl;
			break;

		case 3:
			cout << "�������ļ����� : " << endl;
			cin >> filename;
			if (!print_cnf(filename)) {
				cout << "���ļ�ʧ��." << endl;
				break;
			}
			break;

		case 4:
			cout << "���������ʱ������ : ";
			cin >> time_limit;
			cout << "�޶����ʱ��Ϊ :" << time_limit << "ms" << endl;
			break;

		case 5:
			cout << "��ӡ���������� :" << endl;
			solver.show_result();
			break;
		case 7:
			cout << "������ԭcnf�ļ��� :";
			cin >> filename;
			cout << "������������ļ��� :";
			cin >> outname;
			check_solve(filename, outname);
			break;
		case 0:
			return;

		default:
			break;
		}
		cin >> i;
		if (i == 0)
			return;
	} while (!solved_flag);
	return;
}

void print_sudoku_option() {
	system("cls");
	cout << "---------- SUDOKU ----------" << endl;
	cout << "1 : ��������." << endl;
	cout << "2 : չʾ��������." << endl;
	cout << "3 : ����cnf�ļ�." << endl;
	cout << "4 : sat��Ⲣת��Ϊ����." << endl;
	cout << "5 : ��֤�����Ƿ���ȷ." << endl;
	cout << "0 : �˳�." << endl;
}
void choose_suduku_option() {
	int i = 0;
	cout << "������ѡ�� : ";
	bool generated_flag = false;
	FILE *fp = nullptr;
	Sudoku sudoku;
	string filename;
	CDCLSolver solver;
	do {
		cin >> i;
		switch (i) {
			generated_flag = false;
		case 1:
			int complex;
			cout << "�������Ѷ�: 1:���� 2:�е� 3:����" << endl;
			cin >> complex;
			sudoku.generate(complex);
			cout << "���� :";
			sudoku.nholes();
			break;
		case 2:
			sudoku.show_final();
			break;
		case 3:
			cout << "����������ļ����� : " << endl;
			cin >> filename;
			sudoku.transfer_cnf(filename);
			break;
		case 4:
			cout << "��������ļ��� :" << endl;
			cin >> filename;
			fp = fopen(filename.c_str(), "r");
			solver.solve(fp, 100);
			solver.result_to_file("solved-" + filename);
			sudoku.cnf_to_table("solved-" + filename);
			sudoku.show_filled();
			break;
		case 5:
			sudoku.check_file();
			break;
		case 0:
			break;
		}
	} while (i != 0);
}


bool print_cnf(string filename) {
	std::ifstream file(filename);
	if (!file)
		return false;
	std::stringstream s;
	s << file.rdbuf();
	cout << s.str().c_str();
	return true;
}

int main() {
	print_option();
	choose_option();
	return 0;
}
