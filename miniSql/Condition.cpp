#include <iostream>
#include <sstream>
#include "Condition.h"
using namespace std;


bool Condition::ifRight(const int content) const {
	stringstream ss;
	ss << value;
	int myContent;
	ss >> myContent;

	switch (operate) {
	case Condition::OPERATOR_EQUAL:
		return content == myContent;
		break;
	case Condition::OPERATOR_NOT_EQUAL:
		return content != myContent;
		break;
	case Condition::OPERATOR_LESS:
		return content < myContent;
		break;
	case Condition::OPERATOR_MORE:
		return content > myContent;
		break;
	case Condition::OPERATOR_LESS_EQUAL:
		return content <= myContent;
		break;
	case Condition::OPERATOR_MORE_EQUAL:
		return content >= myContent;
		break;
	default:
		return true;
		break;
	}
}

bool Condition::ifRight(const float content) {
	stringstream ss;
	ss << value;
	float myContent;
	ss >> myContent;

	switch (operate) {
	case Condition::OPERATOR_EQUAL:
		return content == myContent;
		break;
	case Condition::OPERATOR_NOT_EQUAL:
		return content != myContent;
		break;
	case Condition::OPERATOR_LESS:
		return content < myContent;
		break;
	case Condition::OPERATOR_MORE:
		return content > myContent;
		break;
	case Condition::OPERATOR_LESS_EQUAL:
		return content <= myContent;
		break;
	case Condition::OPERATOR_MORE_EQUAL:
		return content >= myContent;
		break;
	default:
		return true;
		break;
	}
}

bool Condition::ifRight(const string& content) {
	string myContent = value;
	switch (operate) {
	case Condition::OPERATOR_EQUAL:
		return content == myContent;
		break;
	case Condition::OPERATOR_NOT_EQUAL:
		return content != myContent;
		break;
	case Condition::OPERATOR_LESS:
		return content < myContent;
		break;
	case Condition::OPERATOR_MORE:
		return content > myContent;
		break;
	case Condition::OPERATOR_LESS_EQUAL:
		return content <= myContent;
		break;
	case Condition::OPERATOR_MORE_EQUAL:
		return content >= myContent;
		break;
	default:
		return true;
		break;
	}
}

Condition::Condition(string a, string v, int o) {
	attributeName = a;
	value = v;
	operate = o;
}
