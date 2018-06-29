#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include "API.h"
using namespace std;

class Interpreter {
public:

	Api* ap;
	string fileName;
	int interpreter(const string& sql);

	static string getWord(string sql, int* tmp);

	Interpreter(): ap(new Api()) {
	}

	~Interpreter() = default;
};

#endif
