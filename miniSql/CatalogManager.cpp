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

int CatalogManager::addTable(string TableName, vector<Attribute> *AttributeVector, string PrimaryKey = "",
                             int PKeyLocation = 0) {

    FILE *fp;
    fp = fopen(TableName.c_str(), "w+");
    if (fp == NULL) {
        return 0;
    }
    fclose(fp);
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b != nullptr) {
        char *addressBegin = b->getContent();
        int *size = (int *) addressBegin;
        *size = 0;// 0 record number
        addressBegin += sizeof(int);
        *addressBegin = PKeyLocation;//1 as what it says
        addressBegin++;
        *(addressBegin) = AttributeVector->size();// 2 attribute number
//        char a = *addressBegin;
        addressBegin++;
//        memcpy(addressBegin, attributeVector, (*attributeVector).size()*sizeof(Attribute));
        for (int i = 0; i < (*AttributeVector).size(); i++) {
            memcpy(addressBegin, &((*AttributeVector)[i]), sizeof(Attribute));
            addressBegin += sizeof(Attribute);
        }
        b->setUsingSize(b->getUsingSize() + (*AttributeVector).size() * sizeof(Attribute) + 2 + sizeof(int));
        b->setDirty();
        return 1;
    }
    return 0;


}

int CatalogManager::dropTable(string TableName) {
    bm.deleteFileNode(TableName.c_str());
    if (remove(TableName.c_str())) {
        return 0;
    }
    return 1;
}

int CatalogManager::addIndex(string IndexName, string TableName, string Attribute, int Type) {
    FileNode *f = bm.fetchFileNode("Indexs");
    BlockNode *b = bm.fetchBlockHead(f);
    IndexInfo i(IndexName, TableName, Attribute, Type);
    while (true) {
        if (b == NULL) {
            return 0;
        }
        if (b->getUsingSize() <= bm.getBlockSize() - sizeof(IndexInfo)) {

            char *addressBegin;
            addressBegin = b->getContent() + b->getUsingSize();
            memcpy(addressBegin, &i, sizeof(IndexInfo));
            b->setUsingSize(b->getUsingSize() + sizeof(IndexInfo));
            b->setDirty();


            return this->setAttributeIndex(TableName, Attribute, IndexName);
        } else {
            b = bm.fetchNextBlock(f, b);
        }
    }

    return 0;
}

int CatalogManager::setAttributeIndex(string TableName, string AttributeName, string IndexName) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {

        char *addressBegin = b->getContent();
        addressBegin += 1 + sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *) addressBegin;
        int i;
        for (i = 0; i < size; i++) {
            if (a->name == AttributeName) {
                a->index = IndexName;
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


int CatalogManager::dropIndex(string Index) {
    FileNode *f = bm.fetchFileNode("Indexs");
    BlockNode *b = bm.fetchBlockHead(f);
    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        IndexInfo *i = (IndexInfo *) addressBegin;
        int j = 0;
        for (j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
            if ((*i).indexName == Index) {
                break;
            }
            i++;
        }
        this->deleteAttributeIndex((*i).tableName, (*i).Attribute, (*i).indexName);
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


int CatalogManager::deleteAttributeIndex(string TableName, string AttributeName, string IndexName) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {

        char *addressBegin = b->getContent();
        addressBegin += (1 + sizeof(int));
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *) addressBegin;
        int i;
        for (i = 0; i < size; i++) {
            if (a->name == AttributeName) {
                if (a->index == IndexName) {
                    a->index = "";
                    b->setDirty();
                } else {
                    cout << "revoke unknown index: " << IndexName << " on " << TableName << "!" << endl;
                    cout << "Attribute: " << AttributeName << " on table " << TableName << " has index: " << a->index
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


int CatalogManager::findTable(string TableName) {
    FILE *fp;
    fp = fopen(TableName.c_str(), "r");
    if (fp == NULL) {
        return 0;
    } else {
        fclose(fp);
        return TABLE_FILE;
    }

}


int CatalogManager::findIndex(string IndexName) {
    FileNode *f = bm.fetchFileNode("Indexs");
    BlockNode *b = bm.fetchBlockHead(f);
    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        IndexInfo *i = (IndexInfo *) addressBegin;
        int flag = UNKNOWN_FILE;
        for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
            if ((*i).indexName == IndexName) {
                flag = INDEX_FILE;
                break;
            }
            i++;
        }
        return flag;
    }

    return 0;
}

int CatalogManager::insertRecord(string TableName, int RecordNum) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {

        char *addressBegin = b->getContent();
        int *originalRecordNum = (int *) addressBegin;
        *originalRecordNum += RecordNum;
        b->setDirty();
        return *originalRecordNum;
    }
    return 0;
}

int CatalogManager::deleteRecord(string TableName, int DeleteNum) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {

        char *addressBegin = b->getContent();
        int *recordNum = (int *) addressBegin;
        if ((*recordNum) < DeleteNum) {
            cout << "error in CatalogManager::deleteValue" << endl;
            return 0;
        } else
            (*recordNum) -= DeleteNum;

        b->setDirty();
        return *recordNum;
    }
    return 0;
}

int CatalogManager::getRecordNum(string TableName) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {
        char *addressBegin = b->getContent();
        int *recordNum = (int *) addressBegin;
        return *recordNum;
    }
    return 0;
}

int CatalogManager::getIndexNameList(string TableName, vector<string> *IndexNameVector) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        IndexInfo *i = (IndexInfo *) addressBegin;
        for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
            if ((*i).tableName == TableName) {
                (*IndexNameVector).push_back((*i).indexName);
            }
            i++;
        }
        return 1;
    }

    return 0;
}

int CatalogManager::getAllIndex(vector<IndexInfo> *Indexs) {
    FileNode *f = bm.fetchFileNode("Indexs");
    BlockNode *b = bm.fetchBlockHead(f);
    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        IndexInfo *i = (IndexInfo *) addressBegin;
        for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
            Indexs->push_back((*i));
            i++;
        }
    }

    return 1;
}

int CatalogManager::getIndexType(string IndexName) {
    FileNode *f = bm.fetchFileNode("Indexs");
    BlockNode *b = bm.fetchBlockHead(f);
    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        IndexInfo *i = (IndexInfo *) addressBegin;
        for (int j = 0; j < (b->getUsingSize() / sizeof(IndexInfo)); j++) {
            if ((*i).indexName == IndexName) {
                return i->type;
            }
            i++;
        }
        return -2;
    }

    return -2;
}


void CatalogManager::getRecordString(string TableName, vector<string> *RecordContent, char *RecordResult) {
    vector<Attribute> attributeVector;
    getAttribute(TableName, &attributeVector);
    char *contentBegin = RecordResult;

    for (int i = 0; i < attributeVector.size(); i++) {
        Attribute attribute = attributeVector[i];
        string content = (*RecordContent)[i];
        int type = attribute.type;
        int typeSize = calculateLength2(type);
        stringstream ss;
        ss << content;
        if (type == Attribute::TYPE_INT) {
            //if the content is a int
            int intTmp;
            ss >> intTmp;
            memcpy(contentBegin, ((char *) &intTmp), typeSize);
        } else if (type == Attribute::TYPE_FLOAT) {
            //if the content is a float
            float floatTmp;
            ss >> floatTmp;
            memcpy(contentBegin, ((char *) &floatTmp), typeSize);
        } else {
            //if the content is a string
            memcpy(contentBegin, content.c_str(), typeSize);
        }

        contentBegin += typeSize;
    }
    return;
}

int CatalogManager::getAttribute(string TableName, vector<Attribute> *attributeVector) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);
    if (b) {
        char *addressBegin;
        addressBegin = b->getContent();
        addressBegin += 1 + sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *) addressBegin;
        for (int i = 0; i < size; i++) {
            attributeVector->push_back(*a);
            a ++;
        }

        return 1;
    }
    return 0;
}

int CatalogManager::calculateLength(string TableName) {
    FileNode *f = bm.fetchFileNode(TableName.c_str());
    BlockNode *b = bm.fetchBlockHead(f);

    if (b) {
        int singleRecordSize = 0;
        char *addressBegin = b->getContent();
        addressBegin += 1 + sizeof(int);
        int size = *addressBegin;
        addressBegin++;
        Attribute *a = (Attribute *) addressBegin;
        for (int i = 0; i < size; i++) {
            if ((*a).type == -1) {
                singleRecordSize += sizeof(float);
            } else if ((*a).type == 0) {
                singleRecordSize += sizeof(int);
            } else if ((*a).type > 0) {
                singleRecordSize += (*a).type * sizeof(char);
            } else {
                cout << "Catalog data damaged!" << endl;
                return 0;
            }
            a++;
        }

        return singleRecordSize;
    }
    return 0;
}

int CatalogManager::calculateLength2(int Type) {
    if (Type == Attribute::TYPE_INT) {
        return sizeof(int);
    } else if (Type == Attribute::TYPE_FLOAT) {
        return sizeof(float);
    } else {
        return (int) sizeof(char)*Type; // Note that the type stores in Attribute.h
    }
}
