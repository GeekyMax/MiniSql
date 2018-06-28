#include "IndexManager.h"
#include "BufferManager.h"
#include "BPlusTree.h"

extern BufferManager bm;

int degree(int keysize) {
//    return 5;
    int degree = BLOCK_SIZE / (keysize + sizeof(int));
    if (degree % 2 == 0) degree -= 1;
    return degree;
}

int keysize(typeenum type) {
    if (type == INT) return sizeof(int);
    if (type == FLOAT) return sizeof(float);
    return 0;
}

IndexManager::IndexManager() {

}

IndexManager::~IndexManager() {
//    return;
    for (map<string, tree *>::iterator i = treemap.begin(); i != treemap.end(); i++) {
        i->second->write();
        delete i->second;
    }
}

int IndexManager::read(string filename) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->read();
}

int IndexManager::write(string filename) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->write();
}

int IndexManager::create(string filename, typeenum type, int size) {
    tree *t = new tree(filename, degree(size), type, size);
    treemap.insert(pair<string, tree *>(filename, t));
    return 0;
}

int IndexManager::drop(string filename) {
    if (!treemap.count(filename)) return -1;

    delete treemap[filename];
    treemap.erase(filename);
    return 0;
}

int IndexManager::search(string filename, string key) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->search(key);
}

int IndexManager::insert(string filename, string key, int val) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->insert(key, val);
}

int IndexManager::deleta(string filename, string key) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->deleta(key);
//    return 0;
}

int IndexManager::print(string filename) {
    if (!treemap.count(filename)) return -1;

    return treemap[filename]->print();
}
