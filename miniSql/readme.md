#Buffer Manager部分

## 一、各类接口

### 1.1 BlockNode

Block 是数据块的基本单位，大小等于BLOCK_SIZE，此宏在BufferManager.h中定义。Block的第一个字节用于存储该Block的using size。

```c++
char* getContent();// 返回Block的存储地址，用于数据块的读取与存储
void setdirty(); // 设置本Block是否是dirty，是否需要write back
void cleanDirty(); 
void setUsingSize(); // 设置本BlocK的usingSize
size_t getUsingSize();
void setPin(bool isPin); //锁定本Block，避免被回收，传入flase也可以解锁

```

### 1.2 FileNode

文件节点，每个File Node，各自再维护其block node list.

```c++
void setPin(bool isPin); // 锁定或解锁该文件
```



### 1.3 BufferManager

Buffer Manager类负责缓冲区的管理，对file pool 和block pool的一些管理，根据需求对缓冲区块进行替换。

```c++
FileNode *fetchFileNode(const char *fileName, bool isPin=false); //获取一个文件节点。如果该文件节点已经在buffer中就返回。如过没有，就从file pool中获取一个空块返回。如果file pool已满，就回收其他陈旧的块，清空并设置，返回。
void deleteFileNode(const char* fileName);// 删除指定文件块，包括其下的所有block node.
BlockNode *fetchNextBlock(FileNode *file,BlockNode *block);// 获取指定block node的下一块，如果不存在就在之后添加一块。如果block是nullptr，就直接插在file的BlockHead上。
BlockNode *fetchBlockHead(FileNode *file); // 获取文件头一块block，不存在的话同上。
BlockNode *fetchBlockByOffset(FileNode *file, int offset); // 通过node的offset来获取。同上。
int getBlockSize()； //获取Block可用的总空间。

```

## 二、基本使用

### 1.插入内容

```c++
//example 1: 插入内容
BufferManager bm;
FileNode *fileNode = bm.fetchFile(fileName);// 获得fileNode
BlockNode *blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
// 循环遍历节点block链表
while(true){
    if(blockNode!=nullptr){
        if(blockNode->getUsingSize() <= bm.getBlockSize() - recordSize) //判断该block是否有空
        {
            char* address = blockNode.getContent()+blockNode.getUsingSize(); // 得到空余空间的地址
            memcpy(address,record,recordSize); // 将记录存入
            blockNode.setDirty(); // block已修改，设置dirty
            blockNode.setUsingSize(blockNode.getUsingSize()+recordSize); // 更新block的使用空间
        }
        else // 如果该节点空间不够就找下一个节点。
        {
            blockNode = bm.fetchNextBlock(fileNode,blockNode);
        }
    }else{
        break;
    }
}
```

### 