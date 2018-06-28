#include "API.h"
#include <sstream>
#include "RecordManager.h"
#include "CatalogManager.h"
#include "IndexManager.h"

#define UNKNOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10

CatalogManager *cm;
IndexManager* im;

/*
* drop a table
* @param tableName: name of table
*/
void API::DropTable(string tableName)
{
	if (!tableExist(tableName)) return;
	vector<string> indexNameVector;
	int i;
	//get all index in the table, and drop them all
	indexNameListGet(tableName, &indexNameVector);
	for (i = 0; i < indexNameVector.size(); i++)
	{
		printf("%s", indexNameVector[i].c_str());
		indexDrop(indexNameVector[i]);
	}
	//delete a table file
	if (rm->tableDrop(tableName))
	{
		//delete a table information
		cm->dropTable(tableName);
		printf("Drop table %s successfully!\n", tableName.c_str());
	}
}

/*
* drop a index
* @param indexName: name of index
*/
void API::indexDrop(string indexName)
{
	if (cm->findIndex(indexName) != INDEX_FILE)
	{
		printf("There is no index named %s \n", indexName.c_str());
		return;
	}
	//delete a index file
	if (rm->indexDrop(indexName))
	{
		//get type of index
		int indexType = cm->getIndexType(indexName);
		if (indexType == -2)
		{
			printf("Error!\n");
			return;
		}
		//delete a index information
		cm->dropIndex(indexName);
		//delete a index tree
		im->drop(rm->getIndexFileName(indexName));
		printf("Drop index %s successfully!\n", indexName.c_str());
	}
}

/*
* create a index
* @param indexName: name of index
* @param tableName: name of table
* @param attributeName: name of attribute in a table
*/
void API::indexCreate(string indexName, string tableName, string attributeName)
{
	if (cm->findIndex(indexName) == INDEX_FILE)
	{
		cout << "There is index " << indexName << " already!" << endl;
		return;
	}
	if (!tableExist(tableName)) return;
	int i, indexType;
	int type = 0;
	vector<Attribute> attributeVector;
	cm->getAttribute(tableName, &attributeVector);
	for (i = 0; i < attributeVector.size(); i++)
	{
		if (attributeName == attributeVector[i].name)
		{
			if (!attributeVector[i].ifUnique)
			{
				cout << "The attribute is not unique!" << endl;
				return;
			}
			type = attributeVector[i].type;
			break;
		}
	}
	if (i == attributeVector.size())
	{
		cout << "There is no such attribute in the table!" << endl;
		return;
	}
	//RecordManager to create a index file
	if (rm->indexCreate(indexName))
	{
		//CatalogManager to add a index information
		cm->addIndex(indexName, tableName, attributeName, type);
		//get type of index
		indexType = cm->getIndexType(indexName);
		if (indexType == -2)
		{
			cout << "Error!";
			return;
		}
		//indexManager to create a index tress
		if (indexType == -1) {
			im->create(rm->getIndexFileName(indexName), FLOAT, sizeof(float));
		}
		else if (indexType == 0) {
			im->create(rm->getIndexFileName(indexName), INT, sizeof(int));
		}
		else {
			cout << "error";
			return;
		}															//recordManager insert already record to index
		rm->indexRecordAlreadyInsert(tableName, indexName);
		printf("Create index %s successfully!\n", indexName.c_str());
	}
	else
	{
		cout << "Create index " << indexName << " fails!" << endl;
	}
}

/*
* create a table
* @param tableName: name of table
* @param attributeVector: vector of attribute
* @param primaryKeyName: primary key of a table (default: "")
* @param primaryKeyLocation: the primary position in the table
*/
void API::tableCreate(string tableName, vector<Attribute>* attributeVector, string primaryKeyName, int primaryKeyLocation)
{
	if (cm->findTable(tableName) == TABLE_FILE)
	{
		cout << "There is a table named " << tableName << " already!" << endl;
		return;
	}
	//RecordManager to create a table file
	if (rm->tableCreate(tableName))
	{
		//CatalogManager to create a table information
		cm->addTable(tableName, attributeVector, primaryKeyName, primaryKeyLocation);
		printf("Create table %s successfully!\n", tableName.c_str());
	}
	if (primaryKeyName != "")
	{
		//get a primary key
		string indexName = "PRIMARY_" + tableName;
		indexCreate(indexName, tableName, primaryKeyName);
	}
}
/*
* show all record of attribute in the table and the number of the record
* @param tableName: name of table
* @param attributeNameVector: vector of name of attribute
*/
void API::recordShow(string tableName, vector<string>* attributeNameVector)
{
	vector<Condition> conditionVector;
	recordShow(tableName, attributeNameVector, &conditionVector);
}

/*
* show the record matching the coditions of attribute in the table and the number of the record
* @param tableName: name of table
* @param attributeNameVector: vector of name of attribute
* @param conditionVector: vector of condition
*/
void API::recordShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector)
{
	if (cm->findTable(tableName) == TABLE_FILE)
	{
		int num = 0, blockOffset = -1;
		vector<Attribute> attributeVector;
		vector<string> allAttributeName;
		attributeGet(tableName, &attributeVector);
		if (attributeNameVector == NULL) {
			for (Attribute attribute : attributeVector)
			{
				allAttributeName.insert(allAttributeName.end(), attribute.name);
			}
			attributeNameVector = &allAttributeName;
		}
		//print attribute name you want to show
		AttributePrint(attributeNameVector);
		for (string name : (*attributeNameVector))
		{
			int i;
			for (i = 0; i < attributeVector.size(); i++)
			{
				if (attributeVector[i].name == name)
				{
					break;
				}
			}
			if (i == attributeVector.size())
			{
				cout << "The attribute is not exist in the table!" << endl;
				return;
			}
		}
		if (conditionVector != NULL)
		{
			for (Condition condition : *conditionVector)
			{
				int i = 0;
				for (i = 0; i < attributeVector.size(); i++)
				{
					if (attributeVector[i].name == condition.attributeName)
					{
						if (condition.operate == Condition::OPERATOR_EQUAL && attributeVector[i].index != "")
						{
							blockOffset = im->search(rm->getIndexFileName(attributeVector[i].index), condition.value);
						}
						break;
					}
				}
				if (i == attributeVector.size())
				{
					cout << "The attribute is not exist in the table!" << endl;
					return;
				}
			}
		}

		if (blockOffset == -1)
		{
			num = rm->recordShow(tableName, attributeNameVector, conditionVector);
		}
		else
		{
			//find the block by index,search in the block
			num = rm->recordShowWithIndex(tableName, attributeNameVector, conditionVector, blockOffset);
		}
		printf("%d records selected\n", num);
	}
	else
	{
		cout << "There is no table named " << tableName << endl;
	}
}

/**
*
* insert a record to a table
* @param tableName: name of table
* @param recordContent: Vector of these content of a record
*/
void API::recordInsert(string tableName, vector<string>* recordContent)
{
	if (!tableExist(tableName)) return;
	int i;
	string indexName;
	//deal if the record could be insert (if index is exist)
	vector<Attribute> attributeVector;
	vector<Condition> conditionVector;

	attributeGet(tableName, &attributeVector);
	for (i = 0; i < attributeVector.size(); i++)
	{
		indexName = attributeVector[i].indexNameGet();
		if (indexName != "")
		{
			//if the attribute has a index
			int blockoffest = im->search(rm->getIndexFileName(indexName), (*recordContent)[i]);
			if (blockoffest != -1)
			{
				//if the value has exist in index tree then fail to insert the record
				cout << "Insert fails because index value already exists!" << endl;
				return;
			}
		}
		else if (attributeVector[i].ifUnique)
		{
			//if the attribute is unique but not index
			Condition condition(attributeVector[i].name, (*recordContent)[i], Condition::OPERATOR_EQUAL);
			conditionVector.insert(conditionVector.end(), condition);
		}
	}

	if (conditionVector.size() > 0)
	{
		for (i = 0; i < conditionVector.size(); i++) {
			vector<Condition> conditionTmp;
			conditionTmp.insert(conditionTmp.begin(), conditionVector[i]);

			int recordConflictNum = rm->findAllRecords(tableName, &conditionTmp);
			if (recordConflictNum > 0) {
				cout << "Insert fails because unique value exists!" << endl;
				return;
			}

		}
	}
	char recordString[2000];
	memset(recordString, 0, 2000);
	//CatalogManager to get the record string
	cm->getRecordString(tableName, recordContent, recordString);
	//RecordManager to insert the record into file; and get the position of block being insert
	int recordSize = cm->calculateLength(tableName);
	int blockOffset = rm->recordInsert(tableName, recordString, recordSize);

	if (blockOffset >= 0)
	{
		recordIndexInsert(recordString, recordSize, &attributeVector, blockOffset);
		cm->insertRecord(tableName, 1);
		printf("Insert record into table %s successful!\n", tableName.c_str());
	}
	else
	{
		cout << "Insert record into table " << tableName << " fails!" << endl;
	}
}

/*
* delete all record of table
* @param tableName: name of table
*/
void API::recordDelete(string tableName)
{
	vector<Condition> conditionVector;
	recordDelete(tableName, &conditionVector);
}

/*
* delete the record matching the coditions in the table
* @param tableName: name of table
* @param conditionVector: vector of condition
*/
void API::recordDelete(string tableName, vector<Condition>* conditionVector)
{
	if (!tableExist(tableName)) return;

	int num = 0, blockOffset = -1;
	vector<Attribute> attributeVector;
	attributeGet(tableName, &attributeVector);
	if (conditionVector != NULL)
	{
		for (Condition condition : *conditionVector)
		{
			if (condition.operate == Condition::OPERATOR_EQUAL)
			{
				for (Attribute attribute : attributeVector)
				{
					if (attribute.index != "" && attribute.name == condition.attributeName)
					{
						blockOffset = im->search(rm->getIndexFileName(attribute.index), condition.value);
					}
				}
			}
		}
	}

	if (blockOffset == -1)
	{
		//if we con't find the block by index,we need to find all block
		num = rm->deleteAllRecords(tableName, conditionVector);
	}
	else
	{
		//find the block by index,search in the block
		num = rm->deleteAllRecordsByIndex(tableName, conditionVector, blockOffset);
	}
	//delete the number of record in the table
	cm->deleteRecord(tableName, num);
	printf("Delete %d record in table %s\n", num, tableName.c_str());
}

/*
* get the size of a record in table
* @param tableName: name of table
*/
int API::recordSizeGet(string tableName)
{
	if (!tableExist(tableName)) return 0;
	return cm->calculateLength(tableName);
}
/*
* get the size of a type
* @param type:  type of attribute
*/
int API::typeSizeGet(int type)
{
	return cm->calculateLength2(type);
}
/*
* get the vector of a all name of index in the table
* @param tableName:  name of table
* @param indexNameVector:  a point to vector of indexName(which would change)
*/
int API::indexNameListGet(string tableName, vector<string>* indexNameVector)
{
	if (!tableExist(tableName)) {
		return 0;
	}
	return cm->getIndexNameList(tableName, indexNameVector);
}

/*
* get the vector of all name of index's file
* @param indexNameVector: will set all index's
*/
void API::allIndexAddressInfoGet(vector<IndexInfo> *indexNameVector)
{
	cm->getAllIndex(indexNameVector);
	for (int i = 0; i < (*indexNameVector).size(); i++)
	{
		(*indexNameVector)[i].indexName = rm->getIndexFileName((*indexNameVector)[i].indexName);
	}
}

/*
* get the vector of a attribute type in a table
* @param tableName:  name of table
* @param attributeNameVector:  a point to vector of attributeType(which would change)
*/
int API::attributeGet(string tableName, vector<Attribute>* attributeVector)
{
	if (!tableExist(tableName)) {
		return 0;
	}
	return cm->getAttribute(tableName, attributeVector);
}

/*
* insert all index value of a record to index tree
* @param recordBegin: point to record begin
* @param recordSize: size of the record
* @param attributeVector:  a point to vector of attributeType(which would change)
* @param blockOffset: the block offset num
*/
void API::recordIndexInsert(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, int blockOffset)
{
	char* contentBegin = recordBegin;
	int i;
	for (i = 0; i < (*attributeVector).size(); i++)
	{
		int type = (*attributeVector)[i].type;
		int typeSize = typeSizeGet(type);
		if ((*attributeVector)[i].index != "")
		{
			indexInsert((*attributeVector)[i].index, contentBegin, type, blockOffset);
		}
		contentBegin += typeSize;
	}
}

/*
* insert a value to index tree
* @param indexName: name of index
* @param contentBegin: address of content
* @param type: the type of content
* @param blockOffset: the block offset num
*/
void API::indexInsert(string indexName, char* contentBegin, int type, int blockOffset)
{
	string content = "";
	stringstream tmp;
	//if the attribute has index
	if (type == Attribute::TYPE_INT)
	{
		int value = *((int*)contentBegin);
		tmp << value;
	}
	else if (type == Attribute::TYPE_FLOAT)
	{
		float value = *((float*)contentBegin);
		tmp << value;
	}
	else
	{
		char value[255];
		memset(value, 0, 255);
		memcpy(value, contentBegin, sizeof(type));
		string stringTmp = value;
		tmp << stringTmp;
	}
	tmp >> content;
	im->insert(rm->getIndexFileName(indexName), content, blockOffset);
}

/*
* delete all index value of a record to index tree
* @param recordBegin: point to record begin
* @param recordSize: size of the record
* @param attributeVector:  a point to vector of attributeType(which would change)
* @param blockOffset: the block offset num
*/
void API::recordIndexDelete(char* recordBegin, int recordSize, vector<Attribute>* attributeVector, int blockOffset)
{
	char* contentBegin = recordBegin;
	int i;
	for (i = 0; i < (*attributeVector).size(); i++)
	{
		int type = (*attributeVector)[i].type;
		int typeSize = typeSizeGet(type);
		string content = "";
		stringstream tmp;

		if ((*attributeVector)[i].index != "")
		{
			//if the attribute has index
			if (type == Attribute::TYPE_INT)
			{
				int value = *((int*)contentBegin);
				tmp << value;
			}
			else if (type == Attribute::TYPE_FLOAT)
			{
				float value = *((float*)contentBegin);
				tmp << value;
			}
			else
			{
				char value[255];
				memset(value, 0, 255);
				memcpy(value, contentBegin, sizeof(type));
				string stringTmp = value;
				tmp << stringTmp;
			}

			tmp >> content;
			im->deleta(rm->getIndexFileName((*attributeVector)[i].index), content);

		}
		contentBegin += typeSize;
	}

}

/*
* check if the table exists
* @param tableName the name of the table
*/
int API::tableExist(string tableName)
{
	if (cm->findTable(tableName) != TABLE_FILE)
	{
		cout << "There is no such table named " << tableName << endl;
		return 0;
	}
	else
	{
		return 1;
	}
}
/**
* print attribute name
* @param attributeNameVector: the vector of attribute's name
*/
void API::AttributePrint(vector<string>* attributeNameVector)
{
	int i;
	for (i = 0; i < (*attributeNameVector).size(); i++)
	{
		printf("%s ", (*attributeNameVector)[i].c_str());
	}
	if (i != 0)
		printf("\n");
}

