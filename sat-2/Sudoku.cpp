#include "Sudoku.h"
#include <cassert>
using namespace std;

void Sudoku::generate(int complexity) {
	srand((unsigned)time(NULL));
	int holenum;
	if (complexity == 1) {
		holenum = rand() % 9 + 30;
	}
	else if (complexity == 2) {
		holenum = rand() % 9 + 40;
	}
	else if (complexity == 3) {
		holenum = 60;
	}
	else {
		cout << "请输入难度" << endl;
		return;
	}

	set_holeNum(holenum);
	initialize();
	show_table(1);
}
/*
//问题：是否可能出现有的洞一旦挖了后面怎么挖都出现多解的情况？如何解决？跳出循环？
void Sudoku::dig_unique_solution() {
	int tmpX, tmpY, tmp;
	int checkX, checkY, checkNum;
	vector<int> holes;
	int tmptable[9][9] = { 0 };
	tmp = rand() % 81;
	tmpX = tmp / 9;
	tmpY = tmp % 9;
	this->_sudokuTable.holetable[tmpX][tmpY] = 0;
	holes.push_back(tmp);
	//后面每挖一个洞，都要判断解的唯一性
	while (holes.size() != this->_sudokuTable.holeNum) {
		tmp = rand() % 81;
		tmpX = tmp / 9;
		tmpY = tmp % 9;
		if (this->_sudokuTable.holetable[tmpX][tmpY] != 0) { //看洞是不是原来挖过
			this->_sudokuTable.holetable[tmpX][tmpY] = 0;    //先放进去，如果不成立在吐出来
			holes.push_back(tmp);
		}
		else
			continue;

		//检查挖的洞是否有冲突
		for (int i = 0; i < holes.size(); i++) {
			int checkNum = holes[i];
			int checkX = checkNum / 9;
			int checkY = checkNum % 9;
			for (int val = 0; val < 9; val++) {
				if (check_conllision(checkX, checkY, val, this->_sudokuTable.holetable) && val != this->_sudokuTable.table[checkX][checkY]) {
					this->_sudokuTable.holetable[tmpX][tmpY] = this->_sudokuTable.table[tmpX][tmpY];
					tmptable[checkX][checkY]++;
					if (tmptable[checkX][checkY] > 100) {
						holes.erase(holes.begin() + i);
						cout << checkNum << "是冲突的罪魁祸首,把他删了" << endl;
						memset(tmptable[0], 0, 81 * 4);
					}
					holes.pop_back();
					break;
				}
			}
		}
	}
}
*/

void Sudoku::dig_holes() {
	uniform_int_distribution<int> choose_branch(0, 8);
	//srand((unsigned)time(NULL));
	for (int i = 0; i < _sudokuTable.holeNum;) {

		int tmpX = choose_branch(generator);
		int tmpY = choose_branch(generator);
		/*int tmpX = rand() % 9;
		int tmpY = rand() % 9;*/
		if (_sudokuTable.holetable[tmpX][tmpY] == 0); //如果挖到重复的洞，则不操作
		else {
			i++;
			_sudokuTable.holetable[tmpX][tmpY] = 0; //在挖洞的地方填上0
		}
	}
}

void Sudoku::set_holeNum(int holeNum) {
	_sudokuTable.holeNum = holeNum;
}

bool Sudoku::check_conllision(int x, int y, int val, int sudotable[][9]) {
	//检测是否填充
	if (sudotable[x][y] != 0)
		return false;
	//检测单行冲突
	for (int j = 0; j < 9; j++) {
		if (val == sudotable[x][j])
			return false;
	}
	//检测单列冲突
	for (int i = 0; i < 9; i++) {
		if (val == sudotable[i][y])
			return false;
	}
	//检测格冲突
	int tmpRow = x / 3 * 3;
	int tmpCol = y / 3 * 3;
	for (int i = tmpRow; i < tmpRow + 3; i++) {
		for (int j = tmpCol; j < tmpCol + 3; j++) {
			if (val == sudotable[i][j])
				return false;
		}
	}
	return true;
}

void Sudoku::show_table(int table_model) {  //0:终盘 1:待解盘
	int(*tmp_array)[9];
	if (table_model == 0) {
		tmp_array = _sudokuTable.table;
	}
	else
		tmp_array = _sudokuTable.holetable;
	cout << "----------------------------" << endl;
	for (int i = 0; i < 9; i++) {
		cout << "|";
		for (int j = 0; j < 9; j++) {
			cout << tmp_array[i][j] << " |";
		}
		cout << endl << "----------------------------" << endl;
	}
}

void Sudoku::initialize() {
	fill_table(0, 1);
	//将原完整数独拷贝给待挖洞数独
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			_sudokuTable.holetable[i][j] = _sudokuTable.table[i][j];
		}
	}

	int tmptable[9][9] = { 0 };

	while (true) {
		dig_holes();
		if (check_mul_solution()) {
			break;
		}
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				_sudokuTable.holetable[i][j] = _sudokuTable.table[i][j];
			}
		}
	}
}

bool Sudoku::fill_table(int x, int val) {
	//生成一个随机1-9序列
	uniform_int_distribution<int> choose_branch(0, 8);
	int n, tmp;
	int row[9];
	for (int i = 0; i < 9; i++) {
		row[i] = i;
	}
	for (int i = 0; i < 9; i++) {
		n = choose_branch(generator);
		tmp = row[n];
		row[n] = row[i];
		row[i] = tmp;
	}
	//用回溯法生成一个初始化填充完毕的数独
	for (int i = 0; i < 9; i++) {
		int y = row[i];
		if (check_conllision(x, y, val, _sudokuTable.table)) {
			_sudokuTable.table[x][y] = val;
			if (x == 8) {
				if (val == 9 || fill_table(0, val + 1))
					return true;
			}
			else if (fill_table(x + 1, val))
				return true;
			_sudokuTable.table[x][y] = 0; //回溯
		}
	}
	return false;
}

//将一个矩阵转化为cnf，如果文字已经赋值
//那么就添加他的单元文字
bool Sudoku::transfer_cnf(string filename) {
	fstream file(filename, ios::in | ios::out | ios::trunc);
	if (!file)
		return false;
	file << "p cnf 729 " << flush; //cnf格式：729表示729个文字，是固定的
	int clauses = (81 - _sudokuTable.holeNum) + 10449;//10449表示出去单元子句外的子句数
	file << clauses << endl;

	//0：将已经填有数字的格子导入为单元子句
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (_sudokuTable.holetable[i][j] != 0) {
				file << 81 * i + 9 * j + _sudokuTable.holetable[i][j] << " 0" << endl;
			}
		}
	}

	//1：格子内数字在1-9之间
	//<(1, 1）x1> | <(1, 2) x2> | … | <(1, 1) x9> * 81
	for (int i = 0; i < 81; i++) {
		for (int j = 0; j < 9; j++) {
			file << i * 9 + j + 1 << ' ';
		}
		file << '0' << endl;
	}

	//2:一个格子里面不能取两个数字
	//<(1,1)!x1> | <(1,1)!x2> <(1,1)!x1> | <(1,1)!x3>…
	//<(1,1)!x1> | <(1,1)!x9>
	for (int i = 0; i < 9; i++) {
		for (int j = i; j < 9; j++) {
			for (int a = 0; a < 9; a++) {
				for (int b = a + 1; b < 9; b++) {
					file << -1 * (i * 81 + j * 9 + a + 1) << ' ';
					file << -1 * (i * 81 + j * 9 + b + 1) << " 0" << endl;
				}
			}
		}
	}
	//3:行之间的合理性
	//<(i,1)x1> ^ <(i,2)x1> <(i,1)x2> ^ <(i,2)x2>…
	//<(i,1)x1> ^ <(i,2)x9>
	//同一个九宫格内行列在格冲突中已经输出，此处不用输出
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			for (int jj = j + 1; jj < 9; jj++) {
				for (int a = 0; a < 9; a++) {
					file << -1 * (i * 81 + j * 9 + a + 1) << ' ';
					file << -1 * (i * 81 + jj * 9 + a + 1) << " 0" << endl;
				}
			}
		}
	}
	//4:列之间的合理性
	for (int i = 0; i < 9; i++) {
		for (int ii = i + 1; ii < 9; ii++) {
			for (int j = 0; j < 9; j++) {
				for (int a = 0; a < 9; a++) {
					file << -1 * (i * 81 + j * 9 + a + 1) << ' ';
					file << -1 * (ii * 81 + j * 9 + a + 1) << " 0" << endl;
				}
			}
		}
	}
	//5:格之间的合理性
	for (int t = 0; t < 3; t++) {
		for (int i = t * 3; i < 3 * t + 3; i++) {
			for (int tt = 0; tt < 3; tt++) {
				for (int j = tt * 3; j < 3 * tt + 3; j++) {
					for (int ii = i; ii < (i / 3) * 3 + 3; ii++) {
						for (int jj = (j / 3) * 3; jj < (j / 3) * 3 + 3; jj++) {
							for (int a = 0; a < 9; a++) {
								if ((i != ii || j != jj) && (jj > j || ii > i)) {
									file << -1 * (i * 81 + j * 9 + a + 1) << ' ';
									file << -1 * (ii * 81 + jj * 9 + a + 1) << " 0" << endl;
								}
							}
						}
					}
				}
			}
		}
	}
	return true;
}
bool Sudoku::check_file() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (_sudokuTable.table[i][j] != _sudokuTable.holetable[i][j]) {
				cout << "(" << i << " " << j << endl;
				cout << "not correct" << endl;
				return false;
			}
		}
	}
	cout << "correct" << endl;
	return true;
}

bool Sudoku::cnf_to_table(string filename) {
	ifstream file(filename, ios::in | ios::out | ios::app);
	if (!file)
		return false;
	char buf[10];
	char buff[10000];
	int literal;
	stringstream ss;
	string tmp;
	getline(file, tmp);
	file.get(buf, 3);
	ss << file.rdbuf();
	while (true) {
		ss >> literal;
		if (literal == 0)
			break;
		if (literal > 0) {
			literal--;
			int val = literal % 9 + 1;
			int tmpX = (literal - val + 1) / 81;
			int tmpY = (literal - val - 81 * tmpX + 1) / 9;
			_sudokuTable.holetable[tmpX][tmpY] = val;
		}
	}
	return true;
}

//暂时做挖洞单一解用
bool Sudoku::check_mul_solution() {
	int num = 0;
	backtrace_solve(0, num);
	return num <= 1;
}

void Sudoku::backtrace_solve(int count, int &num) {
	if (count == 80) {
		++num;
		return;
	}
	int row = count / 9;
	int col = count % 9;
	if (_sudokuTable.holetable[row][col] == 0) {
		for (int i = 1; i <= 9; i++) {
			if (check_conllision(row, col, i, _sudokuTable.holetable)) {
				_sudokuTable.holetable[row][col] = i;
				backtrace_solve(count + 1, num);
				if (num > 1)
					return;
				_sudokuTable.holetable[row][col] = 0;
			}
		}
	}
	else {
		backtrace_solve(count + 1, num);
		if (num > 1)
			return;
	}
	return;
}
