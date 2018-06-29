//
// Created by h2279 on 2018/06/18.
//

#ifndef MINISQL_BUFFERMANAGER_H
#define MINISQL_BUFFERMANAGER_H

#include <iostream>
#include "Minisql.h"

#define BLOCK_SIZE 4096
#define MAX_FILE_NUM 10
#define MAX_BLOCK_NUM 40
#define MAX_FILE_NAME_LENGTH 100

static int replacedBlock = -1;

class BlockNode {
public:
	bool isBottom;
	int offsetNum;

	/**
	 * the default constructor
	 * init the fields and set the content memory to 0
	 */
	BlockNode();

	/**
	 * the default destructor, delete the name memory and content memory
	 */
	~BlockNode();

	/**
	 * set the block dirty when a block's content has been changed
	 */
	void setDirty();

	void cleanDirty();

	/**
	 * the setter od usingSize
	 * @param usingSize
	 */
	void setUsingSize(size_t usingSize);

	/**
	 * the getter of usingSize
	 * @return
	 */
	size_t getUsingSize();

	/**
	 * the setting of pin
	 * if you want to pin a block to avoid the recycling. you should set the pin true.
	 * @param isPin
	 */
	void setPin(bool isPin);

	/**
	 * get the content address of the block except the block head
	 * @return
	 */
	char* getContent();

	friend class BufferManager;

	char* address;
private:
	char* fileName;
	bool pin;
	bool reference;
	BlockNode* nextNode;
	BlockNode* preNode;
	bool dirty;
	size_t usingSize;

	void init();

	/**
	 * write the all the content to the disk unless it's not dirty.
	 */
	void writeBackToDisk();

	/**
	 * update the usingSize when you read a block from the disk.
	 */
	void updateUsingSize() {
		this->usingSize = *(size_t *)address;
	}
};

class FileNode {
public:
	FileNode();

	~FileNode();

	/**
	 * meaningless maybe..
	 * @param isPin
	 */
	void setPin(bool isPin);

	friend class BufferManager;

private:
	char* fileName;
	bool pin;
	FileNode* nextNode;
	FileNode* preNode;
	BlockNode* blockHead;

	void init();
};

class BufferManager {
public:
	BufferManager();

	~BufferManager();

	/**
	 * fetch a file node by filename.
	 * if this file already in the list, return it
	 * else if there are enough space in the pool, make one and return it.
	 * if there are no space in the pool, replace some block
	 * @param fileName the name of the file
	 * @param isPin if you want pin this file node, input true
	 * @return the file node you want
	 */
	FileNode* fetchFileNode(const char* fileName, bool isPin = false);

	/**
	 * delete the file node and release all its blocks.
	 * @param fileName the name of file you want to delete.
	 */
	void deleteFileNode(const char* fileName);

	/**
	 * try to fetch the next block sfter the giving block. if there are no block in the list
	 * after this block, system will append a new block after your input block and return it.
	 * if this block is bottom ,fetching will not happen
	 * @param file the file node your block belongs to
	 * @param block the current block
	 * @return the next block after your input block, unless your input is the bottom of the file.
	 */
	BlockNode* fetchNextBlock(FileNode* file, BlockNode* block);

	/**
	 * fetch the blockhead
	 * if the block head doesn't exist ,append one/
	 * @param file
	 * @return
	 */
	BlockNode* fetchBlockHead(FileNode* file);

	/**
	 * fecth a block by offset number
	 * @param file the file node your block belongs to
	 * @param offset the offset number of your goal block.
	 * @return
	 */
	BlockNode* fetchBlockByOffset(FileNode* file, int offset);

	/**
	 * get the block's fixed size except the head.
	 * @return
	 */
	static int getBlockSize() //Get the size of the block that others can use.Others cannot use the block head
	{
		return BLOCK_SIZE - sizeof(size_t);
	}

	//private:
	FileNode* fileHead;
	FileNode filePool[MAX_FILE_NUM];
	BlockNode blockPool[MAX_BLOCK_NUM];
	int totalBlock;
	int totalFile;

	/**
	 * fetch a block in the given position.
	 * if the block is already in the list, return it.
	 * if the block not in the list, replace some fileNode, using LRU replacement, if required, to make more space.
	 * Only if the block is dirty, he will be written back to disk when been replaced.
	 * @param file  the file you want to add the block into.
	 * @param position  the position that the Node will added to.
	 * @param isPin if true, the block will be locked.
	 * @return BockNode*
	 */
	BlockNode* fetchBlock(FileNode* file, BlockNode* position, bool isPin = false);

	/**
	 * flush all block node in the list to the disk.
	 */
	void writeBackToDiskAll();

};

static BufferManager bm;

#endif //MINISQL_BUFFERMANAGER_H
