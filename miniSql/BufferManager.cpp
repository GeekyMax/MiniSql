//
// Created by h2279 on 2018/06/18.
//

#include "BufferManager.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <queue>

BlockNode::BlockNode() {
    // init the content memory
    address = new char[BLOCK_SIZE];
    fileName = new char[MAX_FILE_NAME_LENGTH];
    init();

}

void BlockNode::init() {
    dirty = false;
    usingSize = sizeof(size_t);
    nextNode = preNode = nullptr;
    pin = false;
    reference = false;
    isBottom = false;
    offsetNum = -1;
    if (address == nullptr) {
        printf("Can not allocate memory in the initialization of  the block pool!\n");
        exit(1);
    }
    memset(address,
           0, BLOCK_SIZE);
    size_t initUsage = 0;
    memcpy(address,
           (char *) &initUsage, sizeof(size_t)); // 设置block的头部用于存储usage
// init the file name
    if (fileName == nullptr) {
        printf("Can not allocate memory in the initialization of the block pool!\n");
        exit(1);
    }
    memset(fileName, 0, MAX_FILE_NAME_LENGTH);
}

BlockNode::~BlockNode() {
    delete[] fileName;
    delete[] address;
}

void BlockNode::setUsingSize(size_t usingSize) {
    BlockNode::usingSize = usingSize;
    memcpy(address, (char *) &usingSize, sizeof(size_t));
}

size_t BlockNode::getUsingSize() {
    return usingSize;
}

char *BlockNode::getContent() {
    return address + sizeof(size_t);
}

void BlockNode::setPin(bool isPin) {
    pin = isPin;
    if (!pin) reference = true;
}

void BlockNode::setDirty() {
    this->dirty = true;
}

void BlockNode::cleanDirty() {
    this->dirty = false;
}

void BlockNode::writeBackToDisk() {
    if (!dirty) // this block is not been modified, so it do not need to written back to files
    {
        return;
    } else // written back to the file
    {
        FILE *fileHandle = nullptr;
        if ((fileHandle = fopen(fileName, "rb+")) != nullptr) {
            if (fseek(fileHandle, offsetNum * BLOCK_SIZE, 0) == 0) {
                if (fwrite(address, usingSize + sizeof(size_t), 1, fileHandle) != 1) {
                    printf("Problem writing the file %s in writtenBackToDisk", fileName);
                    exit(1);
                }
            } else {
                printf("Problem seeking the file %s in writtenBackToDisk", fileName);
                exit(1);
            }
            fclose(fileHandle);
        } else {
            printf("Problem opening the file %s in writtenBackToDisk", fileName);
            exit(1);
        }
    }
}

FileNode::FileNode() {
    fileName = new char[MAX_FILE_NAME_LENGTH];
    init();
}

FileNode::~FileNode() {
    delete[] fileName;
}

void FileNode::init() {
    nextNode = preNode = nullptr;
    blockHead = nullptr;
    pin = false;
    memset(fileName, 0, MAX_FILE_NAME_LENGTH);

}

void FileNode::setPin(bool isPin) {
    pin = isPin;
}

BufferManager::BufferManager() : totalBlock(0), totalFile(0), fileHead(nullptr) {
}

BufferManager::~BufferManager() {
    writeBackToDiskAll();
}

FileNode *BufferManager::fetchFileNode(const char *fileName, bool isPin) {
    FileNode *thisFile;
    if (fileHead != nullptr) {
        for (FileNode *fileNode = fileHead; fileNode != nullptr; fileNode = fileNode->nextNode) {
            if (!strcmp(fileName, fileNode->fileName)) {
                fileNode->pin = isPin;
                return fileNode;
            }
        }
    }
    // if not in the list
    if (totalFile == 0) {
        thisFile = fileHead = &filePool[0];
        totalFile++;
    } else if (totalFile < MAX_FILE_NUM) {
        thisFile = &filePool[totalFile];
        thisFile->preNode = &filePool[totalFile - 1];
        filePool[totalFile - 1].nextNode = thisFile;
        totalFile++;
    } else {
        thisFile = fileHead;
        while (thisFile->pin) {
            if (thisFile->nextNode) {
                thisFile = thisFile->nextNode;
            } else {
                printf("There are no enough file node in the pool!");
                exit(2);
            }
        }
        BlockNode *nextBlock;
        for (BlockNode *block = thisFile->blockHead; block != nullptr; block = nextBlock) {
            nextBlock = block->nextNode;
            block->writeBackToDisk();
            block->init();
            totalBlock--;
        }
        thisFile->init();
    }

    if (strlen(fileName) >= MAX_FILE_NAME_LENGTH) {
        printf("文件名长度过长，最高不能超过%d\n", MAX_FILE_NAME_LENGTH);
        exit(3);
    }
    strncpy(thisFile->fileName, fileName, MAX_FILE_NAME_LENGTH);
    thisFile->pin = isPin;
    return thisFile;
}

BlockNode *BufferManager::fetchBlock(FileNode *file, BlockNode *position, bool isPin) {
    BlockNode *thisBlock = nullptr;
    if (totalBlock == 0) {
        thisBlock = blockPool + 1;
        totalBlock++;
    } else if (totalBlock < MAX_BLOCK_NUM) {
        for (int i = 0; i < MAX_BLOCK_NUM; ++i) {
            if (blockPool[i].offsetNum < 0) {
                thisBlock = &blockPool[i];
                totalBlock++;
                break;
            }
        }
    } else {
        int i = replacedBlock;
        while (true) {
            i++;
            if (i >= MAX_BLOCK_NUM) i = 0;
            if (blockPool[i].pin) {
                continue;
            }
            if (blockPool[i].reference) {
                blockPool[i].reference = false;
            } else {
                thisBlock = &blockPool[i];
                replacedBlock = i;
                if (thisBlock->nextNode != nullptr) {
                    thisBlock->nextNode->preNode = thisBlock;
                }
                if (thisBlock->preNode != nullptr) {
                    thisBlock->preNode->nextNode = thisBlock;
                }
                if (file->blockHead == thisBlock) {
                    file->blockHead = thisBlock->nextNode;
                }
                thisBlock->writeBackToDisk();
                thisBlock->init();
                break;
            }
        }
    }
    if (position != nullptr) {
        if (position->nextNode != nullptr) {
            position->nextNode->preNode = thisBlock;
        }
        thisBlock->nextNode = position->nextNode;
        position->nextNode = thisBlock;
        thisBlock->offsetNum = position->offsetNum + 1;
    } else {
        if (file->blockHead != nullptr) {
            thisBlock->nextNode = file->blockHead;
            file->blockHead->preNode = thisBlock;
        }
        thisBlock->offsetNum = 0;
        file->blockHead = thisBlock;
    }
    thisBlock->setPin(isPin);
    if (strlen(file->fileName) >= MAX_FILE_NAME_LENGTH) {
        printf("文件名长度过长，最高不能超过%d\n", MAX_FILE_NAME_LENGTH);
        exit(3);
    }
    strncpy(thisBlock->fileName, file->fileName, MAX_FILE_NAME_LENGTH);
    // load the content from the file
    FILE *fileHandle;
    if ((fileHandle = fopen(file->fileName, "ab+")) != nullptr) {
        if (fseek(fileHandle, thisBlock->offsetNum * BLOCK_SIZE, 0) == 0) {
            if (fread(thisBlock->address, 1, BLOCK_SIZE, fileHandle) == 0) { // 0 means read eof.
                thisBlock->isBottom = true;
            }
            thisBlock->updateUsingSize();
        } else {
            printf("Problem seeking the file %s in reading", file->fileName);
            exit(1);
        }
        fclose(fileHandle);
    } else {
        printf("Problem opening the file %s in reading", file->fileName);
        exit(1);
    }
    return thisBlock;
}


void BufferManager::writeBackToDiskAll() {
    BlockNode *nextNode = nullptr;
    if (fileHead == nullptr) return;
    for (FileNode *fileNode = fileHead; fileNode != nullptr; fileNode = fileNode->nextNode) {
        if (fileNode->blockHead != nullptr) {
            for (BlockNode *blockNode = fileNode->blockHead; blockNode != nullptr; blockNode = nextNode) {
                nextNode = blockNode->nextNode;
                blockNode->writeBackToDisk();
                blockNode->init();
            }
        }
    }
}

BlockNode *BufferManager::fetchNextBlock(FileNode *file, BlockNode *block) {
    if (block->nextNode == nullptr) {
        if (block->isBottom) block->isBottom = false;
        return fetchBlock(file, block);
    } else //block->nextBlock != NULL
    {
        if (block->offsetNum == block->nextNode->offsetNum - 1) {
            return block->nextNode;
        } else //the block list is not in the right order
        {
            return fetchBlock(file, block);
        }
    }
}

BlockNode *BufferManager::fetchBlockHead(FileNode *file) {
    BlockNode *thisBlock = nullptr;
    if (file->blockHead != nullptr) {
        if (file->blockHead->offsetNum == 0) {
            thisBlock = file->blockHead;
        } else {
            thisBlock = fetchBlock(file, nullptr);
        }
    } else {
        thisBlock = fetchBlock(file, nullptr);
    }
    return thisBlock;
}

BlockNode *BufferManager::fetchBlockByOffset(FileNode *file, int offset) {
    BlockNode *thisBlock = nullptr;
    if (offset == 0) thisBlock = fetchBlockHead(file);
    else {
        thisBlock = fetchBlockHead(file);
        while (offset > 0) {
            offset--;
            thisBlock = fetchNextBlock(file, thisBlock);
        }
    }
    return thisBlock;
}

void BufferManager::deleteFileNode(const char *fileName) {
    FileNode *file = fetchFileNode(fileName);
    BlockNode *thisBlock = fetchBlockHead(file);
    BlockNode *nextBlock = nullptr;
    std::queue<BlockNode *> blockQueue;
    while (true) {
        if (thisBlock == nullptr) {
            return;
        }
        blockQueue.push(thisBlock);
        if (thisBlock->isBottom) break;
        thisBlock = fetchNextBlock(file, thisBlock);
    }

    while (!blockQueue.empty()) {
        blockQueue.front()->init();
        blockQueue.pop();
    }
    if (file->preNode != nullptr) file->preNode->nextNode = file->nextNode;
    if (file->nextNode != nullptr) file->nextNode->preNode = file->preNode;
    if (fileHead == file) fileHead = file->nextNode;
    file->init();
    totalFile--;
}