#ifndef MINISQL_CONDITION_H
#define MINISQL_CONDITION_H
#include <cstring>
#include <iostream>
using namespace std;

class Condition {

public:
	const static int OPERATOR_EQUAL = 0; // "="
	const static int OPERATOR_NOT_EQUAL = 1; // "<>"
	const static int OPERATOR_LESS = 2; // "<"
	const static int OPERATOR_MORE = 3; // ">"
	const static int OPERATOR_LESS_EQUAL = 4; // "<="
	const static int OPERATOR_MORE_EQUAL = 5; // ">="

	Condition(string a, string v, int o);

	string attributeName;
	string value; // the value to be compared
	int operate; // the type to be compared

	bool ifRight(int content);
	bool ifRight(float content);
	bool ifRight(const string& content);
};

#endif
