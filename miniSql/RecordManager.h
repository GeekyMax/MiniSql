#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include "Condition.h"
#include "Attribute.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include "Minisql.h"
#include <string>
#include <vector>

#define MAX_SIZE 255
using namespace std;

class Api;

class RecordManager {
public:
	RecordManager() {
	}

	BufferManager bm;
	Api* api;

	bool tableCreate(string tableName); //create tablefile
	bool tableDrop(string tableName); //drop tablefile

	bool indexCreate(string indexName); //create indexfile
	bool indexDrop(string indexName); //drop indexfile

	int recordInsert(string tableName, char* record, int recordSize); //insert record into files

	int recordShow(string tableName, vector<string>* attributeNameVector,
	               vector<Condition>* conditionVector); //find all the records that meet the conditions
	int recordShowWithIndex(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector,
	                        int blockOffset); //find all the records that meet the conditions with index

	int findAllRecords(string tableName,
	                   vector<Condition>* conditionVector); //count the number of the records meet the conditions

	int deleteAllRecords(string tableName,
	                     vector<Condition>* conditionVector); // delete all the records that meet the conditions
	int deleteAllRecordsByIndex(string tableName, vector<Condition>* conditionVector,
	                            int blockOffset); //delete all the records that meet the conditions with index

	int indexRecordAlreadyInsert(string tableName, string indexName); //give the inserted records index

	string getTableFileName(string tableName); //get the filename of the table
	string getIndexFileName(string indexName); //get the filename of the index
private:
	int recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector,
	                    BlockNode* block);

	int recordBlockFind(string tableName, vector<Condition>* conditionVector, BlockNode* block);

	int recordBlockDelete(string tableName, vector<Condition>* conditionVector, BlockNode* block);

	int indexRecordBlockAlreadyInsert(string tableName, string indexName, BlockNode* block);

	bool recordConditionFit(char* recordBegin, int recordSize, vector<Attribute>* attributeVector,
	                        vector<Condition>* conditionVector);

	void recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector,
	                 vector<string>* attributeNameVector);

	bool contentConditionFit(char* content, int type, Condition* condition);

	void contentPrint(char* content, int type);
};

#endif
