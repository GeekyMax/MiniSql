#ifndef MINISQL_INDEX_INFO_H
#define MINISQL_INDEX_INFO_H

#include <string>
using namespace std;

class IndexInfo {
public:
	IndexInfo(string i, string t, string a, int ty) {
		indexName = i;
		tableName = t;
		attribute = a;
		type = ty;
	}

	string indexName;
	string tableName;
	string attribute;
	int type;
};

#endif
