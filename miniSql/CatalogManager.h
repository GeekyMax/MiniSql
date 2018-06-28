//
//  CatalogManager.hpp
//  MiniSQL
//
//  Created by HoraceCai on 2018/6/23.
//  Copyright © 2018年 apple. All rights reserved.
//

#ifndef CatalogManager_hpp
#define CatalogManager_hpp


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

    CatalogManager() {};

    ~CatalogManager() {};

    int addTable(string TableName, vector<Attribute> *AttributeVector, string PrimaryKey, int PKeyLocation);

    int dropTable(string TableName);

    int addIndex(string IndexName, string TableName, string AttributeName, int Type);

    int setAttributeIndex(string TableName, string AttributeName, string IndexName);

    int dropIndex(string Index);

    int deleteAttributeIndex(string TableName, string AttributeName, string IndexName);

    int findTable(string TableName);

    int findIndex(string IndexName);

    int insertRecord(string TableName, int RecordNum);

    int deleteRecord(string TableName, int DeleteNum);

    int getRecordNum(string TableName);

    int getIndexNameList(string TableName, vector<string> *IndexNameVector);

    int getAllIndex(vector<IndexInfo> *Indexs);

    int getIndexType(string IndexName);

    void getRecordString(string TableName, vector<string> *RecordContent, char *RecordResult);

    int calculateLength(string TableName);

    int calculateLength2(int Type);


    int getAttribute(string TableName, vector<Attribute> *attributeVector);

};


#endif /* CatalogManager_hpp */
