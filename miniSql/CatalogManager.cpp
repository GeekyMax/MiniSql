//
//  CatalogManager.cpp
//  MiniSQL
//
//  Created by HoraceCai on 2018/6/23.
//  Copyright © 2018年 apple. All rights reserved.
//

#include "CatalogManager.h"
#include "BufferManager.h"
#include "IndexInfo.h"
#include <string>
#include "Attribute.h"
#include <vector>
#include <sstream>

#define TABLE_FILE 9
#define UNKNOWN_FILE 8
#define INDEX_FILE 10

int CatalogManager::addTable(string tableName, vector<Attribute>* attributeVector,
                             int pKeyLocation = 0) {

	FILE* fp = fopen(tableName.c_str(), "w+");
	if (fp == nullptr) {
		return 0;
	}
	fclose(fp);
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b != nullptr) {
		char* addressBegin = b->getContent();
		int* size = (int *)addressBegin;
		*size = 0; // 0 record number
		addressBegin += sizeof(int);
		*addressBegin = pKeyLocation; //1 as what it says
		addressBegin++;
		*(addressBegin) = attributeVector->size(); // 2 attribute number
		//        char a = *addressBegin;
		addressBegin++;
		//        memcpy(addressBegin, attributeVector, (*attributeVector).size()*sizeof(Attribute));
		for (int i = 0; i < (*attributeVector).size(); i++) {
			memcpy(addressBegin, &((*attributeVector)[i]), sizeof(Attribute));
			addressBegin += sizeof(Attribute);
		}
		b->setUsingSize(b->getUsingSize() + (*attributeVector).size() * sizeof(Attribute) + 2 + sizeof(int));
		b->setDirty();
		return 1;
	}
	return 0;


}

int CatalogManager::dropTable(string tableName) {
	bm.deleteFileNode(tableName.c_str());
	if (remove(tableName.c_str())) {
		return 0;
	}
	return 1;
}

int CatalogManager::addIndex(const string& indexName, const string& tableName, const string& attribute, int type) {
	FileNode* f = bm.fetchFileNode("Indexs");
	BlockNode* b = bm.fetchBlockHead(f);
	IndexInfo i(indexName, tableName, attribute, type);
	while (true) {
		if (b == nullptr) {
			return 0;
		}
		if (b->getUsingSize() <= bm.getBlockSize() - sizeof(IndexInfo)) {

			char * addressBegin = b->getContent() + b->getUsingSize();
			memcpy(addressBegin, &i, sizeof(IndexInfo));
			b->setUsingSize(b->getUsingSize() + sizeof(IndexInfo));
			b->setDirty();


			return this->setAttributeIndex(tableName, attribute, indexName);
		}
		else {
			b = bm.fetchNextBlock(f, b);
		}
	}

	return 0;
}

int CatalogManager::setAttributeIndex(const string& tableName, const string& attributeName, const string&
                                      indexName) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {

		char* addressBegin = b->getContent();
		addressBegin += 1 + sizeof(int);
		int size = *addressBegin;
		addressBegin++;
		Attribute* a = (Attribute *)addressBegin;
		int i;
		for (i = 0; i < size; i++) {
			if (a->name == attributeName) {
				a->index = indexName;
				b->setDirty();
				break;
			}
			a++;
		}
		if (i < size)
			return 1;
		else
			return 0;
	}
	return 0;
}


int CatalogManager::dropIndex(const string& index) {
	FileNode* f = bm.fetchFileNode("Indexs");
	BlockNode* b = bm.fetchBlockHead(f);
	if (b) {
		char * addressBegin = b->getContent();
		IndexInfo* i = (IndexInfo *)addressBegin;
		int j = 0;
		for (j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
			if ((*i).indexName == index) {
				break;
			}
			i++;
		}
		this->deleteAttributeIndex((*i).tableName, (*i).attribute, (*i).indexName);
		for (; j < (b->getUsingSize() / sizeof(IndexInfo) - 1); j++) {
			(*i) = *(i + sizeof(IndexInfo));
			i++;
		}
		b->setUsingSize(b->getUsingSize() - sizeof(IndexInfo));
		b->setDirty();

		return 1;
	}

	return 0;
}


int CatalogManager::deleteAttributeIndex(const string& tableName, const string& attributeName, const string& indexName) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {

		char* addressBegin = b->getContent();
		addressBegin += (1 + sizeof(int));
		int size = *addressBegin;
		addressBegin++;
		Attribute* a = (Attribute *)addressBegin;
		int i;
		for (i = 0; i < size; i++) {
			if (a->name == attributeName) {
				if (a->index == indexName) {
					a->index = "";
					b->setDirty();
				}
				else {
					cout << "revoke unknown index: " << indexName << " on " << tableName << "!" << endl;
					cout << "Attribute: " << attributeName << " on table " << tableName << " has index: " << a->index
						<< "!" << endl;
				}
				break;
			}
			a++;
		}
		if (i < size)
			return 1;
		else
			return 0;
	}
	return 0;
}


int CatalogManager::findTable(const string& tableName) {
	FILE* fp;
	fp = fopen(tableName.c_str(), "r");
	if (fp == NULL) {
		return 0;
	}
	else {
		fclose(fp);
		return TABLE_FILE;
	}

}


int CatalogManager::findIndex(const string& indexName) {
	FileNode* f = bm.fetchFileNode("Indexs");
	BlockNode* b = bm.fetchBlockHead(f);
	if (b) {
		char * addressBegin = b->getContent();
		IndexInfo* i = (IndexInfo *)addressBegin;
		int flag = UNKNOWN_FILE;
		for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
			if ((*i).indexName == indexName) {
				flag = INDEX_FILE;
				break;
			}
			i++;
		}
		return flag;
	}

	return 0;
}

int CatalogManager::insertRecord(const string& tableName, int recordNum) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {

		char* addressBegin = b->getContent();
		int* originalRecordNum = (int *)addressBegin;
		*originalRecordNum += recordNum;
		b->setDirty();
		return *originalRecordNum;
	}
	return 0;
}

int CatalogManager::deleteRecord(const string& tableName, const int deleteNum) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {

		char* addressBegin = b->getContent();
		int* recordNum = (int *)addressBegin;
		if ((*recordNum) < deleteNum) {
			cout << "error in CatalogManager::deleteValue" << endl;
			return 0;
		}
		else
			(*recordNum) -= deleteNum;

		b->setDirty();
		return *recordNum;
	}
	return 0;
}

int CatalogManager::getRecordNum(const string& tableName) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {
		char* addressBegin = b->getContent();
		int* recordNum = (int *)addressBegin;
		return *recordNum;
	}
	return 0;
}

int CatalogManager::getIndexNameList(const string& tableName, vector<string>* indexNameVector) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {
		char* addressBegin;
		addressBegin = b->getContent();
		IndexInfo* i = (IndexInfo *)addressBegin;
		for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
			if ((*i).tableName == tableName) {
				(*indexNameVector).push_back((*i).indexName);
			}
			i++;
		}
		return 1;
	}

	return 0;
}

int CatalogManager::getAllIndex(vector<IndexInfo>* indexs) {
	FileNode* f = bm.fetchFileNode("Indexs");
	BlockNode* b = bm.fetchBlockHead(f);
	if (b) {
		char* addressBegin;
		addressBegin = b->getContent();
		IndexInfo* i = (IndexInfo *)addressBegin;
		for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
			indexs->push_back((*i));
			i++;
		}
	}

	return 1;
}

int CatalogManager::getIndexType(const string& indexName) {
	FileNode* f = bm.fetchFileNode("Indexs");
	BlockNode* b = bm.fetchBlockHead(f);
	if (b) {
		char* addressBegin;
		addressBegin = b->getContent();
		IndexInfo* i = (IndexInfo *)addressBegin;
		for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
			if ((*i).indexName == indexName) {
				return i->type;
			}
			i++;
		}
		return -2;
	}

	return -2;
}


void CatalogManager::getRecordString(const string& tableName, vector<string>* recordContent, char* recordResult) {
	vector<Attribute> attributeVector;
	getAttribute(tableName, &attributeVector);
	char* contentBegin = recordResult;

	for (int i = 0; i < attributeVector.size(); i++) {
		Attribute attribute = attributeVector[i];
		string content = (*recordContent)[i];
		int type = attribute.type;
		int typeSize = calculateLength2(type);
		stringstream ss;
		ss << content;
		if (type == Attribute::TYPE_INT) {
			//if the content is a int
			int intTmp;
			ss >> intTmp;
			memcpy(contentBegin, ((char *)&intTmp), typeSize);
		}
		else if (type == Attribute::TYPE_FLOAT) {
			//if the content is a float
			float floatTmp;
			ss >> floatTmp;
			memcpy(contentBegin, ((char *)&floatTmp), typeSize);
		}
		else {
			//if the content is a string
			memcpy(contentBegin, content.c_str(), typeSize);
		}

		contentBegin += typeSize;
	}
	return;
}

int CatalogManager::getAttribute(const string& tableName, vector<Attribute>* attributeVector) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);
	if (b) {
		char* addressBegin = b->getContent();
		addressBegin += 1 + sizeof(int);
		int size = *addressBegin;
		addressBegin++;
		Attribute* a = (Attribute *)addressBegin;
		for (int i = 0; i < size; i++) {
			attributeVector->push_back(*a);
			a ++;
		}

		return 1;
	}
	return 0;
}

int CatalogManager::calculateLength(const string& tableName) {
	FileNode* f = bm.fetchFileNode(tableName.c_str());
	BlockNode* b = bm.fetchBlockHead(f);

	if (b) {
		int singleRecordSize = 0;
		char* addressBegin = b->getContent();
		addressBegin += 1 + sizeof(int);
		int size = *addressBegin;
		addressBegin++;
		Attribute* a = (Attribute *)addressBegin;
		for (int i = 0; i < size; i++) {
			if ((*a).type == -1) {
				singleRecordSize += sizeof(float);
			}
			else if ((*a).type == 0) {
				singleRecordSize += sizeof(int);
			}
			else if ((*a).type > 0) {
				singleRecordSize += (*a).type * sizeof(char);
			}
			else {
				cout << "Catalog data damaged!" << endl;
				return 0;
			}
			a++;
		}

		return singleRecordSize;
	}
	return 0;
}

int CatalogManager::calculateLength2(const int type) {
	if (type == Attribute::TYPE_INT) {
		return sizeof(int);
	}
	else if (type == Attribute::TYPE_FLOAT) {
		return sizeof(float);
	}
	else {
		return (int)sizeof(char) * type; // Note that the type stores in Attribute.h
	}
}
