#pragma once
#include <vector>
#include <utility>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <set>

#include <algorithm>

const int SUCCESS = 0;
const int FAILURE = 1;

const int LIST_BEGIN_FLAG = -1;

class SimSearcher
{
public:
	SimSearcher();
	~SimSearcher();

	int createIndex(const char *filename, unsigned q);
	int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double> > &result);
	int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned> > &result);

	//int createList(char* item, int id);
	//int initQgrams(const char* word, int word_length, std::vector<char*>& qgrams);
	//int splitWord(const char* line, char c, std::vector<std::string>& words);
	//int insertEDList(char* qgram, int id);
	//int insertJACList(char* item, int id);
	//int createED(char *item, int id);
	//int createJCD(char *item, int id);

	//int calculateED(const char *query, const char* entry, int th);

	int q;
	void printDebug(std::vector<std::pair<unsigned, unsigned> > &result);
};

