#include <iostream>
#include "Interpreter.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include "IndexManager.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "API.h"

void testAll();

void testBufferManager();

void init() {
	FILE *fp;
	fp = fopen("Indexs", "r");
	if (fp == NULL) {
		fp = fopen("Indexs", "w+");
		return;
	}
	fclose(fp);
}

void print() {
	clock_t finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	duration *= 1000;
	printf("now time is %2.1f milliseconds\n", duration * 1000);
}

void testGetWord() {
	Interpreter interpreter;
	int temp = 0;

	while (true) {
		std::string result = interpreter.getWord("select * from book where id=0 and name ='name'", &temp);
		std::cout << result << std::endl << temp;
	}
}

clock_t start;

int main(int argc, char *argv[]) {
	//    testBufferManager();
		testAll();
	//testGetWord();
	//    Attribute a = Attribute("test", 0, false);
	//    Attribute b = Attribute("price", -1, false);
	//    Attribute *m = (Attribute *) malloc(sizeof(Attribute));
	//    memcpy(m, &a, sizeof(Attribute));
	//    std::cout << m->name;
	return 0;
}


void testAll() {
	init();

	API api;
	CatalogManager cm = CatalogManager();
	RecordManager rm = RecordManager();

	api.rm = &rm;
	api.cm = &cm;
	IndexManager im = IndexManager();

	api.im = &im;
	rm.api = &api;

	start = 0;
	clock_t finish;

	cout << "*******************Welcome to use our MiniSQL**********************" << endl;
	cout << "******************* Author: GeekyMax **********************" << endl;
	int fileRead = 0;
	//string fileName ="";
	ifstream file;
	Interpreter in;
	in.ap = &api;
	string s;
	int status = 0;
	while (1) {
		if (fileRead) {

			file.open(in.fileName.c_str());
			if (!file.is_open()) {
				cout << "Fail to open " << in.fileName << endl;
				fileRead = 0;
				continue;
			}
			while (getline(file, s, ';')) {
				in.interpreter(s);
			}
			file.close();
			fileRead = 0;
			finish = clock();
			double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			duration *= 1000;
			printf("%2.1f milliseconds\n", duration);
		}
		else {

			cout << "minisql>>";
			getline(cin, s, ';');
			start = clock();
			status = in.interpreter(s);
			if (status == 2) {
				fileRead = 1;
			}
			else if (status == 587) {
				break;
			}
			else {
				finish = clock();
				double duration = (double)(finish - start) / CLOCKS_PER_SEC;
				duration *= 1000;
				printf("The duration is %2.1f milliseconds\n", duration);
			}
		}

	}
}

void testBufferManager() {
	BufferManager bm = BufferManager();
	FileNode *f = bm.fetchFileNode("good");
	BlockNode *b = bm.fetchBlockHead(f);
	char *content = b->getContent();
	const char *c = "hello word";
	memcpy(content, c, 11);
	printf("%x", content);
	FileNode *f2 = bm.fetchFileNode("good");
	b = bm.fetchBlockHead(f2);
	printf("%x", content);
}

