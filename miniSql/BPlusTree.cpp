#include "BPlusTree.h"
#include "BufferManager.h"
#include <sstream>

int *getaddress(int x)
{
    static int y;
    y=x;
    return &y;
}

float *getaddress(float x)
{
    static float y;
    y=x;
    return &y;
}

char *getaddress(string x)
{
    static string y;
    y=x;
    return (char *)y.c_str();
}

int toint(string key)
{
    istringstream o(key);
    int ans;
    o>>ans;
    return ans;
}

float tofloat(string key)
{
    istringstream o(key);
    float ans;
    o>>ans;
    return ans;
}

template <typename type>
string tostring(type a)
{
    ostringstream o;
    o<<a;
    return o.str();
}

tree::tree(string fileName, int degree, typeenum type, int size): 
    fileName(fileName),
    degree(degree),
    type(type),
    size(size)
{
    switch (type)
    {
        case INT:
        root = new node<int>(NULL, degree, 1);
        this->size = sizeof(int);
        break;

        case FLOAT:
        root = new node<float>(NULL, degree, 1);
        this->size = sizeof(float);
        break;

        case STRING:
        root = new node<string>(NULL, degree, 1);
        break;
    }
}

tree::~tree()
{
    switch (type)
    {
        case INT:
        delete (node<int> *)root;
        break;

        case FLOAT:
        delete (node<float> *)root;
        break;

        case STRING:
        delete (node<string> *)root;
        break;
    }
}

int tree::search(string key)
{
    void *p;
    switch (type)
    {
        case INT:
        p = ((node<int> *)root)->search(toint(key));
        return ((node<int> *)p)->searchval(toint(key));

        case FLOAT:
        p = ((node<float> *)root)->search(tofloat(key));
        return ((node<float> *)p)->searchval(tofloat(key));

        case STRING:
        p = ((node<string> *)root)->search((key));
        return ((node<string> *)p)->searchval((key));
    }
    return 0;
}

int tree::deleta(string key)
{
    void *p;
    switch (type)
    {
        case INT:
        p = ((node<int> *)root)->search(toint(key));
        return ((node<int> *)p)->deleteval(toint(key));

        case FLOAT:
        p = ((node<float> *)root)->search(tofloat(key));
        return ((node<float> *)p)->deleteval(tofloat(key));

        case STRING:
        p = ((node<string> *)root)->search((key));
        return ((node<string> *)p)->deleteval((key));
    }
    return 0;
}

int tree::insert(string key, int val)
{
    void *p;
    switch (type)
    {
        case INT:
        p = ((node<int> *)root)->search(toint(key));
        return ((node<int> *)p)->insert(toint(key), val);

        case FLOAT:
        p = ((node<float> *)root)->search(tofloat(key));
        return ((node<float> *)p)->insert(tofloat(key), val);

        case STRING:
        p = ((node<string> *)root)->search((key));
        return ((node<string> *)p)->insert((key), val);
    }
    return 0;
}

int tree::read()
{
    FileNode *fileNode = bm.fetchFileNode(fileName.c_str());// 获得fileNode
    BlockNode *blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点

    char *address = blockNode->getContent();
    int offset = 0;

    while (1)
    {
        while (offset >= blockNode->getUsingSize()) //判断该block是否有空
        {
            if (blockNode->isBottom) return 0;
            blockNode = bm.fetchNextBlock(fileNode, blockNode);
            address = blockNode->getContent();
            offset = 0;
        }

        char a[100];
        int val;

        memcpy(a, address + offset, size); // 将记录存入
        offset += size;
        memcpy(&val, address + offset, sizeof(int)); // 将记录存入
        offset += sizeof(int);

        switch (type)
        {
            case INT:   insert(tostring(*(int *)a), val);   break;
            case FLOAT: insert(tostring(*(float *)a), val);   break;
            case STRING:insert(a, val);   break;
        }
    }
    return 0;
}

int tree::write()
{
    FileNode *fileNode = bm.fetchFileNode(fileName.c_str());// 获得fileNode
    BlockNode *blockNode = bm.fetchBlockHead(fileNode); // 先拿到头节点
    blockNode->setUsingSize(0);
    blockNode->isBottom=false;
    
    switch (type)
    {
        case INT:
        return ((node<int> *)root)->write(fileNode, blockNode, sizeof(int));

        case FLOAT:
        return ((node<float> *)root)->write(fileNode, blockNode, sizeof(float));

        case STRING:
        return ((node<string> *)root)->write(fileNode, blockNode, size);
    }

    blockNode->isBottom=true;
    blockNode->setDirty();
    return 0;
}

int tree::print()
{
    cout<<"========== d ="<<degree<<endl;
    switch (type)
    {
        case INT:
        return ((node<int> *)root)->print();

        case FLOAT:
        return ((node<float> *)root)->print();

        case STRING:
        return ((node<string> *)root)->print();
    }
    return 0;
}
