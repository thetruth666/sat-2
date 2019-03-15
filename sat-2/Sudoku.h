#ifndef SUDOKU_H
#define SUDOKU_H
#include "common.h"
struct SudokuTable {
	int table[9][9] = { 0 }; //数独盘格
	int holetable[9][9] = { 0 }; //求解棋盘格
	int holeNum = 0; //挖洞数量
};

class Sudoku
{
private:
	SudokuTable _sudokuTable;
	bool fill_table(int cnt, int val);
	bool check_conllision(int x, int y, int val, int sudotable[][9]);
	//void dig_unique_solution();
	void dig_holes();
	void initialize();
	bool check_mul_solution();
	void set_holeNum(int holeNum);
	void show_table(int table_model);
	void backtrace_solve(int count, int &num);
	std::random_device random_generator;
	std::mt19937 generator;
	

public:
	Sudoku() : generator(random_generator()) {}
	void generate(int complexity);   //1简单 2中等 3困难
	void show_final() { show_table(0); };
	void show_filled() { show_table(1); };
	bool transfer_cnf(std::string filename);
	bool cnf_to_table(std::string filename);
	void nholes() { std::cout << _sudokuTable.holeNum << std::endl; };
	bool check_file();
};

#endif