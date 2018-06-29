//
//  CatalogManager.hpp
//  MiniSQL
//
//  Created by HoraceCai on 2018/6/23.
//  Copyright © 2018年 apple. All rights reserved.
//

#ifndef CATALOG_MANAGER_HPP
#define CATALOG_MANAGER_HPP


#include <stdio.h>
#include <string>
#include <vector>
#include "BufferManager.h"
#include "Attribute.h"
#include "IndexInfo.h"

using namespace std;


class CatalogManager {
public:
	BufferManager bm;

	CatalogManager() {
	};

	~CatalogManager() {
	};

	int addTable(string tableName, vector<Attribute>* attributeVector, int pKeyLocation);

	int dropTable(string tableName);

	int addIndex(const string& indexName, const string& tableName, const string& attribute, int type);

	int setAttributeIndex(const string& tableName, const string& attributeName, const string& indexName);

	int dropIndex(const string& index);

	int deleteAttributeIndex(const string& tableName, const string& attributeName, const string& indexName);

	int findTable(const string& tableName);

	int findIndex(const string& indexName);

	int insertRecord(const string& tableName, int recordNum);

	int deleteRecord(const string& tableName, int deleteNum);

	int getRecordNum(const string& tableName);

	int getIndexNameList(const string& tableName, vector<string>* indexNameVector);

	int getAllIndex(vector<IndexInfo>* indexs);

	int getIndexType(const string& indexName);

	void getRecordString(const string& tableName, vector<string>* recordContent, char* recordResult);

	int calculateLength(const string& tableName);

	int calculateLength2(int type);


	int getAttribute(const string& tableName, vector<Attribute>* attributeVector);

};


#endif /* CatalogManager_hpp */
