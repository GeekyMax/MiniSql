#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include "API.h"
using namespace std;
class Interpreter {
public:

	API * ap;
	string fileName;
	int interpreter(string sql);

	string getWord(string sql, int *st);

	Interpreter() {}
	~Interpreter() {}
};

#endif