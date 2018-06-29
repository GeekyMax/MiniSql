#ifndef MINISQL_INDEXMANAGER_H
#define MINISQL_INDEXMANAGER_H

#include <iostream>
#include <string>
#include <map>

using namespace std;

#include "BPlusTree.h"

class IndexManager {
	map<string, tree *> treemap;

public:
	IndexManager();
	~IndexManager();

	int create(string, typeenum, int);
	int drop(string);
	int search(string, string);
	int insert(string, string, int);
	int deleta(string, string);

	int read(string);
	int write(string);
	int print(string);
};

#endif
