#ifndef API_H
#define API_H

#include "Attribute.h"
#include "Condition.h"
#include "IndexInfo.h"
#include <string>
#include <vector>

class CatalogManager;
class RecordManager;
class IndexManager;

class Api {
public:
	RecordManager* rm;
	CatalogManager* cm;
	IndexManager* im;

	Api() {
	}

	~Api() = default;

	void dropTable(string tableName);
	void tableCreate(const string& tableName, vector<Attribute>* attributeVector, const string& primaryKeyName,
	                 int primaryKeyLocation);

	void indexDrop(const string& indexName);
	void indexCreate(const string& indexName, const string& tableName, const string& attributeName);

	void recordShow(string tableName, vector<string>* attributeNameVector = NULL);
	void recordShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector);

	void recordInsert(string tableName, vector<string>* recordContent);

	void recordDelete(string tableName);
	void recordDelete(const string& tableName, vector<Condition>* conditionVector);

	int recordSizeGet(string tableName);
	int typeSizeGet(int type);

	void allIndexAddressInfoGet(vector<IndexInfo>* indexNameVector);
	int attributeGet(string tableName, vector<Attribute>* attributeVector);
	void indexInsert(string indexName, char* contentBegin, int type, int blockOffset);
	void recordIndexDelete(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, int blockOffset);
	void recordIndexInsert(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, int blockOffset);

private:
	int tableExist(const string& tableName);
	int indexNameListGet(string tableName, vector<string>* indexNameVector);
	static void attributePrint(vector<string>* attributeNameVector);
};

#endif
