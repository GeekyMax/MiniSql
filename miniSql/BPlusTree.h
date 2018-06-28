#ifndef MINISQL_BPLUSTREE_H
#define MINISQL_BPLUSTREE_H

#include <iostream>
#include <string>
using namespace std;

#include "BufferManager.h"

typedef enum{INT, FLOAT, STRING} typeenum;

template <typename type>
class node
{
    node *parent;
    node **child;
    type *a;
    int *val;
    bool *del;
    int degree;
    int maxdegree;
    bool leaf;

public:
    node(node *parent, int degree, bool leaf);
    ~node();
    node *search(type);
    int searchval(type);
    int deleteval(type);
    int insert(type, int);
    int insert(type, node *);
    int divide();

    int write(FileNode *, BlockNode *, int);
    int print(int = 0);
};

class tree
{
    string fileName;
    int degree;
    void *root;

public:
    typeenum type;
    int size;

    tree(string fileName, int degree, typeenum type, int size);
    ~tree();

    int search(string key);
    int insert(string key, int val);
    int deleta(string key);

    int read();
    int write();

    int print();
};

template <typename type>
node<type>::node(node *parent, int degree, bool leaf): 
    parent(parent),
    degree(0),
    maxdegree(degree), 
    leaf(leaf)
{
    a = new type[degree+2];
    val = new int[degree+2];
    del = new bool[degree+2];
    child = new node *[degree+2];
    for (int i=0; i<=degree; i++)
    {
        del[i] = 0;
        child[i] = NULL;
    }
}

template <typename type>
node<type>::~node()
{
    for (int i=0; i<=degree; i++)
    {
        if (child[i])
        {
            delete child[i];
        }
    }

    delete[] a;
    delete[] val;
    delete[] del;
    delete[] child;
}

template <typename type>
node<type> *node<type>::search(type key)
{
    if (leaf) return this;

    for (int i=0; i<degree; i++)
    {
        if (key<a[i]) return child[i]->search(key);
    }

    return child[degree]->search(key);
}

template <typename type>
int node<type>::searchval(type key)
{
    for (int i=0; i<this->degree; i++)
    {
        if (key==this->a[i]) return this->val[i];
    }
    return -1;
}

template <typename type>
int node<type>::deleteval(type key)
{
    for (int i=0; i<this->degree; i++)
    {
        if (key==this->a[i]) return this->del[i]=true;
    }
    return 0;
}

template <typename type>
int node<type>::insert(type key, int value)
{
    int t=-1;
    for (int i=0; i<degree; i++)
    {
        if (key<=a[i])
        {
            t=i;
            break;
        }
    }
    if (t==-1) t=degree;
    for (int i=degree; i>t; i--)
    {
        a[i]=a[i-1];
        val[i]=val[i-1];
    }
    a[t]=key;
    val[t]=value;
    degree++;
    
    if (degree > maxdegree)
    {
        divide();
    }
    return 0;
}

template <typename type>
int node<type>::insert(type key, node<type> *p)
{
    int t=-1;
    for (int i=0; i<degree; i++)
    {
        if (key<=a[i])
        {
            t=i;
            break;
        }
    }
    if (t==-1) t=degree;
    for (int i=degree; i>t; i--)
    {
        a[i]=a[i-1];
        child[i+1]=child[i];
    }
    a[t]=key;
    child[t+1]=p;
    degree++;
    
    if (degree > maxdegree)
    {
        divide();
    }
    return 0;
}

template <typename type>
int node<type>::divide()
{
    if (degree < maxdegree) return -1;

    if (parent && leaf)
    {
        node<type> *p = new node<type>(parent, maxdegree, leaf);
        int mid = degree / 2;
        for (int i=0; mid+i<degree; i++)
        {
            p->a[i] = a[mid+i];
            p->val[i] = val[mid+i];
        }

        p->degree = degree - mid;
        degree = mid;

        parent->insert(a[mid], p);
    }

    else if (parent && !leaf)
    {
        node<type> *p = new node<type>(parent, maxdegree, leaf);
        int mid = degree / 2;
        for (int i=0; mid+i+1<degree; i++)
        {
            p->a[i] = a[mid+i+1];
            p->val[i] = val[mid+i+1];
        }
        for (int i=0; mid+i+1<=degree; i++)
        {
            p->child[i] = child[mid+i+1];
            p->child[i]->parent = p;
        }

        p->degree = degree - mid - 1;
        degree = mid;

        parent->insert(a[mid], p);
    }

    else if (!parent && leaf)
    {
        child[0] = new node<type>(this, maxdegree, leaf);
        child[1] = new node<type>(this, maxdegree, leaf);
        leaf = 0;
        child[0]->degree = degree / 2;
        child[1]->degree = degree - child[0]->degree;
        for (int i=0; i<child[0]->degree; i++)
        {
            child[0]->a[i] = a[i];
            child[0]->val[i] = val[i];
        }
        for (int i=0; i+child[0]->degree < degree; i++)
        {
            child[1]->a[i] = a[i+child[0]->degree];
            child[1]->val[i] = val[i+child[0]->degree];
        }

        a[0] = child[1]->a[0];
        degree = 1;
    }

    else if (!parent && !leaf)
    {
        node<type> *p = new node<type>(this, maxdegree, leaf);
        node<type> *q = new node<type>(this, maxdegree, leaf);
        p->degree = degree / 2;
        q->degree = degree - p->degree - 1;
        for (int i=0; i<=p->degree; i++)
        {
            p->a[i] = a[i];
            p->child[i] = child[i];
            p->child[i]->parent = p;
        }
        for (int i=0; i+p->degree+1 <= degree; i++)
        {
            q->a[i] = a[i+p->degree+1];
            q->child[i] = child[i+p->degree+1];
            q->child[i]->parent = q;
        }
        
        a[0] = p->a[p->degree];
        child[0] = p;
        child[1] = q;
        degree = 1;
    }
    return 0;
}

int *getaddress(int x);
float *getaddress(float x);
char *getaddress(string x);

template <typename type>
int node<type>::write(FileNode *fileNode, BlockNode *blockNode, int size)
{
    if (leaf)
    {
        for (int i=0; i<degree; i++)
        {
            if (del[i]) continue;
            if (blockNode)
            {
                if (blockNode->getUsingSize() > bm.getBlockSize() - size - sizeof(int)) //判断该block是否有空
                {
                    blockNode = bm.fetchNextBlock(fileNode, blockNode);
                    blockNode->isBottom=false;
                    blockNode->setUsingSize(0);
                    blockNode->setDirty();
                }

                char *address = blockNode->getContent()+blockNode->getUsingSize(); // 得到空余空间的地址

                memcpy(address, getaddress(a[i]), size); // 将记录存入
                address += size; // 得到空余空间的地址
                memcpy(address, &val[i], sizeof(int)); // 将记录存入

                blockNode->setDirty(); // block已修改，设置dirty
                blockNode->setUsingSize(blockNode->getUsingSize()+size+sizeof(int)); // 更新block的使用空间
            }
            else return -1;
        }
    }
    else
    {
        for (int i=0; i<=degree; i++)
        {
            child[i]->write(fileNode, blockNode, size);
        }
    }
    return 0;
}

template <typename type>
int node<type>::print(int level)
{
    if (leaf)
    {
        for (int i=0; i<degree; i++)
        {
            if (del[i]) continue;
            for (int l=0; l<level; l++) cout<<"    ";
            cout<<"["<<a[i]<<"] = "<<val[i];
            if (parent) cout<<" of "<<parent->a[0];
            cout<<" "<<del[i]<<endl;
        }
    }
    else
    {
        for (int i=0; i<degree; i++)
        {
            child[i]->print(level+1);
            for (int l=0; l<level; l++) cout<<"    ";
            cout<<"["<<a[i]<<"]";
            if (parent) cout<<" of "<<parent->a[0];
            cout<<endl;
        }
        child[degree]->print(level+1);
    }
    return 0;
}

#endif
