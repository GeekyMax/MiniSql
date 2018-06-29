#include <iostream>
#include "RecordManager.h"
#include <cstring>
#include "API.h"

//传入tableName，传出文件名
string RecordManager::getTableFileName(string tableName) {
	string tmp = "";
	return tmp + "Table_File" + tableName;
}

//传入indexName，传出文件名
string RecordManager::getIndexFileName(string indexName) {
	string tmp = "";
	return tmp + "Index_File" + indexName;
}

//建立表文件
bool RecordManager::tableCreate(string tableName) {
	string filename = getTableFileName(tableName);
	FILE* fp = fopen(filename.c_str(), "w+");
	if (fp == NULL) {
		return false;
	}
	fclose(fp);
	return true;
}

//清空此表对应的文件头，并删除文件
bool RecordManager::tableDrop(string tableName) {
	string filename = getTableFileName(tableName);
	bm.deleteFileNode(filename.c_str());
	if (remove(filename.c_str()) == -1) {
		return false;
	}
	else return true;
}

//建立索引文件
bool RecordManager::indexCreate(string indexName) {
	string filename = getIndexFileName(indexName);
	FILE* fp = fopen(filename.c_str(), "w+");
	if (fp == NULL) {
		return false;
	}
	fclose(fp);
	return true;
}

//清空此表对应的文件头，并删除文件
bool RecordManager::indexDrop(string indexName) {
	string filename = getTableFileName(indexName);
	bm.deleteFileNode(filename.c_str());
	if (remove(filename.c_str()) == -1) {
		return false;
	}
	else return true;
}

/**
 * 插入记录
 * tableName为表名
 * record为记录中所有属性的值
 * recordSize为记录大小
 */
int RecordManager::recordInsert(string tableName, char* record, int recordSize) {
	string filename = getTableFileName(tableName);

	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
	// 循环遍历节点block链表
	while (true) {
		if (blockNode != nullptr) {
			if (blockNode->getUsingSize() <= bm.getBlockSize() - recordSize) //判断该block是否有空
			{
				char* address = blockNode->getContent() + blockNode->getUsingSize(); // 得到空余空间的地址
				memcpy(address, record, recordSize); // 将记录存入
				blockNode->setDirty(); // block已修改，设置dirty
				blockNode->setUsingSize(blockNode->getUsingSize() + recordSize); // 更新block的使用空间
				return blockNode->offsetNum;
			}
			else // 如果该节点空间不够就找下一个节点。
			{
				blockNode = bm.fetchNextBlock(fileNode, blockNode);
			}
		}
		else {
			break;
		}
	}
	return -1;
}

/**
 * 选择记录
 * tableName为表名
 * attributeNameVector为存储各属性名的容器
 * conditionVector为存储条件的容器
 */
int
RecordManager::recordShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
	int count = 0;
	while (true) {
		if (blockNode == NULL) {
			return -1;
		}
		else if (blockNode->isBottom) {
			count += recordBlockShow(tableName, attributeNameVector, conditionVector, blockNode);
			return count;
		}
		else {
			count += recordBlockShow(tableName, attributeNameVector, conditionVector, blockNode);
			blockNode = bm.fetchNextBlock(fileNode, blockNode);
		}
	}
}

/**
 * 带有索引的选择记录
 * tableName为表名
 * attributeNameVector为存储各属性名的容器
 * conditionVector为存储条件的容器
 * blockOffset为api部分从index部分取得的块偏移量
 */
int RecordManager::recordShowWithIndex(string tableName, vector<string>* attributeNameVector,
                                       vector<Condition>* conditionVector, int blockOffset) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockByOffset(fileNode, blockOffset); // 先拿到头节点
	int count = 0;
	if (blockNode == NULL) {
		return -1;
	}
	else {
		count += recordBlockShow(tableName, attributeNameVector, conditionVector, blockNode);
		return count;
	}
}

/**
 * 选择记录的块实现
 * tableName为表名
 * attributeNameVector为存储各属性名的容器
 * conditionVector为存储条件的容器
 * block为目前正在检索的块
 */
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector,
                                   vector<Condition>* conditionVector, BlockNode* block) {
	if (block == NULL) {
		return -1;
	}
	int count = 0;
	char* recordBegin = block->getContent();
	vector<Attribute> attributeVector;
	int recordSize = api->recordSizeGet(tableName);

	api->attributeGet(tableName, &attributeVector);
	char* blockBegin = block->getContent();
	size_t usingSize = block->getUsingSize();

	while (recordBegin - blockBegin < usingSize) {
		if (recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector)) {
			count++;
			recordPrint(recordBegin, recordSize, &attributeVector, attributeNameVector);
			printf("\n");
		}
		recordBegin += recordSize;
	}
	return count;
}

/**
 * 查找符合条件记录的数量
 * tableName为表名
 * conditionVector为存储条件的容器
 */
int RecordManager::findAllRecords(string tableName, vector<Condition>* conditionVector) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
	int count = 0;
	while (true) {
		if (blockNode == NULL) {
			return -1;
		}
		else if (blockNode->isBottom) {
			count += recordBlockFind(tableName, conditionVector, blockNode);
			return count;
		}
		else {
			count += recordBlockFind(tableName, conditionVector, blockNode);
			blockNode = bm.fetchNextBlock(fileNode, blockNode);
		}
	}
}

/**
 * 查找符合条件记录的数量的块实现
 * tableName为表名
 * conditionVector为存储条件的容器
 * block为当前正在检索的块
 */
int RecordManager::recordBlockFind(string tableName, vector<Condition>* conditionVector, BlockNode* block) {
	if (block == NULL) {
		return -1;
	}
	int count = 0;
	char* recordBegin = block->getContent();
	vector<Attribute> attributeVector;
	int recordSize = api->recordSizeGet(tableName);

	api->attributeGet(tableName, &attributeVector);
	char* blockBegin = block->getContent();
	size_t usingSize = block->getUsingSize();

	while (recordBegin - blockBegin < usingSize) {
		if (recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector)) {
			count++;
		}
		recordBegin += recordSize;
	}
	return count;
}

/**
 * 记录删除
 * tableName为表名
 * conditionVector为存储条件的容器
 */
int RecordManager::deleteAllRecords(string tableName, vector<Condition>* conditionVector) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
	int count = 0;
	while (true) {
		if (blockNode == NULL) {
			return -1;
		}
		else if (blockNode->isBottom) {
			count += recordBlockDelete(tableName, conditionVector, blockNode);
			return count;
		}
		else {
			count += recordBlockDelete(tableName, conditionVector, blockNode);
			blockNode = bm.fetchNextBlock(fileNode, blockNode);
		}
	}
}

/**
 * 带有索引的记录删除
 * tableName为表名
 * conditionVector为存储条件的容器
 * blockOffset为api从index中取得的块偏移量
 */
int RecordManager::deleteAllRecordsByIndex(string tableName, vector<Condition>* conditionVector, int blockOffset) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockByOffset(fileNode, blockOffset); // 先拿到头节点
	int count = 0;
	if (blockNode == NULL) {
		return -1;
	}
	else {
		count += recordBlockDelete(tableName, conditionVector, blockNode);
		return count;
	}
}

/**
 * 记录删除的块实现
 * tableName为表名
 * conditionVector为存储条件的容器
 * block为正在检索的块
 */
int RecordManager::recordBlockDelete(string tableName, vector<Condition>* conditionVector, BlockNode* block) {
	if (block == NULL) {
		return -1;
	}
	int count = 0;
	char* recordBegin = block->getContent();
	vector<Attribute> attributeVector;
	int recordSize = api->recordSizeGet(tableName);

	api->attributeGet(tableName, &attributeVector);
	char* blockBegin = block->getContent();
	//size_t usingSize = block->getUsingSize();

	while (recordBegin - blockBegin < block->getUsingSize()) {
		// 这个循环令人死亡
		if (recordConditionFit(recordBegin, recordSize, &attributeVector, conditionVector)) {
			count++;
			//这里首先把符合条件的语句用后面的语句覆盖掉，然后把最后空出来的一个元祖清掉
			//因为已经被下一条语句覆盖了，所以不需要再把record的头地址向后移动
			api->recordIndexDelete(recordBegin, recordSize, &attributeVector, block->offsetNum);
			int i = 0;
			for (i = 0; i + recordSize + recordBegin - blockBegin < block->getUsingSize(); i++) {
				recordBegin[i] = recordBegin[i + recordSize];
			}
			memset(recordBegin + i, 0, recordSize);
			block->setUsingSize(block->getUsingSize() - recordSize);
			block->setDirty();
		}
		else
			recordBegin += recordSize;
	}
	return count;
}

/**
 * 判断记录是否符合检索条件
 * recordBegin为存储当前块所有记录的字符指针
 * recordSize为单个记录的大小
 * attributeVector为属性容器，存储了表中各字段的属性
 * conditionVector为条件容器，存储了要求满足的各种条件
 */
bool RecordManager::recordConditionFit(char* recordBegin, int recordSize, vector<Attribute>* attributeVector,
                                       vector<Condition>* conditionVector) {
	if (conditionVector == NULL) {
		return true;
	}
	int type;
	string attributeName;
	int typeSize;
	char content[MAX_SIZE];
	char* contentBegin = recordBegin;

	for (int i = 0; i < attributeVector->size(); i++) {
		type = (*attributeVector)[i].type;
		attributeName = (*attributeVector)[i].name;
		typeSize = api->typeSizeGet(type);

		memset(content, 0, MAX_SIZE);
		memcpy(content, contentBegin, typeSize);
		for (int j = 0; j < (*conditionVector).size(); j++) {
			if ((*conditionVector)[j].attributeName == attributeName) {
				if (!contentConditionFit(content, type, &(*conditionVector)[j])) {
					return false;
				}
			}
		}

		contentBegin += typeSize;
	}
	return true;
}

/**
 * 检查元祖中的某一字段是否符合条件
 * content为该字段的值
 * type为该字段的类型
 * condition为该字段需要满足的条件
 */
bool RecordManager::contentConditionFit(char* content, int type, Condition* condition) {
	if (type == Attribute::TYPE_INT) {
		int tmp = *((int *)content);
		return condition->ifRight(tmp);
	}
	else if (type == Attribute::TYPE_FLOAT) {
		float tmp = *((float *)content);
		return condition->ifRight(tmp);
	}
	else {
		return condition->ifRight(content);
	}
	return true;
}

/**
 * 打印记录
 * recordBegin为存储当前块所有记录的字符指针
 * recordSize为单一记录的大小
 * attributeVector为存储当前表所有字段的属性的容器
 * attributeNameVector为存储当前表所有属性的名字的容器
 */
void RecordManager::recordPrint(char* recordBegin, int recordSize, vector<Attribute>* attributeVector,
                                vector<string>* attributeNameVector) {
	int type;
	string attributeName;
	int typeSize;
	char content[MAX_SIZE];

	char* contentBegin = recordBegin;
	for (int i = 0; i < attributeVector->size(); i++) {
		type = (*attributeVector)[i].type;
		typeSize = api->typeSizeGet(type);
		memset(content, 0, MAX_SIZE);
		memcpy(content, contentBegin, typeSize);
		for (int j = 0; j < (*attributeNameVector).size(); j++) {
			if ((*attributeNameVector)[j] == (*attributeVector)[i].name) {
				contentPrint(content, type);
				break;
			}
		}

		contentBegin += typeSize;
	}
}

/**
 * 打印记录中具体的一个内容
 * content为该记录当前字段的值
 * type为该记录当前字段的类别
 */
void RecordManager::contentPrint(char* content, int type) {
	if (type == Attribute::TYPE_INT) {
		int tmp = *((int *)content);
		printf("%d ", tmp);
	}
	else if (type == Attribute::TYPE_FLOAT) {
		float tmp = *((float *)content);
		printf("%f ", tmp);
	}
	else {
		string tmp = content;
		printf("%s ", tmp.c_str());
	}

}

/**
 * 向已经插入的记录加入索引
 * tableName为表名
 * indexName为索引名
 */
int RecordManager::indexRecordAlreadyInsert(string tableName, string indexName) {
	string filename = getTableFileName(tableName);
	FileNode* fileNode = bm.fetchFileNode(filename.c_str()); // 获得fileNode
	BlockNode* blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
	int count = 0;
	while (true) {
		if (blockNode == nullptr) {
			return -1;
		}
		else if (blockNode->isBottom) {
			count += indexRecordBlockAlreadyInsert(tableName, indexName, blockNode);
			return count;
		}
		else {
			count += indexRecordBlockAlreadyInsert(tableName, indexName, blockNode);
			blockNode = bm.fetchNextBlock(fileNode, blockNode);
		}
	}
}

/**
 * 插入索引的块实现
 * tableName为表名
 * indexName为索引名
 * block为当前正在检索的块
*/
int RecordManager::indexRecordBlockAlreadyInsert(string tableName, string indexName, BlockNode* block) {
	if (block == NULL) {
		return -1;
	}
	int count = 0;
	char* recordBegin = block->getContent();
	vector<Attribute> attributeVector;
	int recordSize = api->recordSizeGet(tableName);

	api->attributeGet(tableName, &attributeVector);
	char* blockBegin = block->getContent();
	size_t usingSize = block->getUsingSize();

	int type;
	int typeSize;
	char* contentBegin;

	while (recordBegin - blockBegin < usingSize) {
		contentBegin = recordBegin;
		for (int i = 0; i < attributeVector.size(); i++) {
			type = attributeVector[i].type;
			typeSize = api->typeSizeGet(type);

			if (attributeVector[i].index == indexName) {
				api->indexInsert(indexName, contentBegin, type, block->offsetNum);
				count++;
			}
			contentBegin += typeSize;
		}
		recordBegin += recordSize;
	}
	return count;
}
