#include "SimSearcher.h"

using namespace std;

#define MAX_NUM 1000000
#define DIS_SIZE 4096

int getMin(int a, int b, int c)
{
	return min(min(a,b),c);
}

struct Line // 输入文件的每一行
{
	char* word;
	int id;
	int length;
};

struct Qgram
{
	char* qgram;
	int index;
};

bool operator<(const Qgram& a,const Qgram& b)
{
	return strcmp(a.qgram, b.qgram) < 0;
}
bool operator==(const Qgram& a,const Qgram& b)
{
	return strcmp(a.qgram, b.qgram) == 0;
}

string string_buf;
char* buffer;
int dis[DIS_SIZE][DIS_SIZE]; //DP算ed距离
int candidate_count[MAX_NUM];
vector<Line> file_lines; 
vector<int> word_num; 
vector<vector<int> > ed_list; //ed的倒排列表
vector<vector<int> > jac_list; //jac的倒排列表
set<Qgram> ed_set;
set<Qgram> jac_set;

int smin = MAX_NUM;


void scanCount(vector<vector<int> >& jac_list, set<Qgram>& jac_set, vector<char*>& query_words, int th, vector<int>& candidate)
{
	candidate.clear();
	for(unsigned i = 0;i < file_lines.size() + 100;i++)
	{
		candidate_count[i] = 0;
	}
    Qgram temp_q;
	for(vector<char*>::iterator it = query_words.begin();it != query_words.end();++it)
	{
		temp_q.qgram = *it;
		set<Qgram>::iterator j = jac_set.find(temp_q);
		if(j == jac_set.end())
			continue;
		vector<int>& temp_list = jac_list[j->index];
		for(vector<int>::iterator k = temp_list.begin();k != temp_list.end();++k)
		{
			candidate_count[*k]++;
			if(candidate_count[*k] == th)
			{
				candidate.push_back(*k);
			}
		}
	}
}

SimSearcher::SimSearcher()
{
}

SimSearcher::~SimSearcher()
{
}

int createList(char* item, int id)
{
	char* new_word = new char[4096];
	strcpy(new_word,item);
	Line e;
	e.word = new_word;
	e.id = id;
	e.length = (int)strlen(new_word);
	file_lines.push_back(e);
	vector<char*> qgrams;
	initQgrams(item, e.length, qgrams);
	for(vector<char*>::iterator it = qgrams.begin();it != qgrams.end();++it)
		insertEDList(*it,id);
	vector<string> words;
	splitWord(item,' ',words);
	word_num.push_back((int)words.size());
	if (smin >  (int)words.size())
		smin = (int)words.size();
	for(vector<string>::iterator it = words.begin();it != words.end();++it)
	{
		char* temp_c = new char[it->length() + 10];
		for(unsigned j = 0;j < it->length();j++)
		{
			temp_c[j] = (*it)[j];
		}
		temp_c[it->length()] = '\0';
		insertJACList(temp_c,id);
	}
	return SUCCESS;
}

int initQgrams(const char* word, int word_length, vector<char*>& qgrams)
{
	qgrams.clear();
	int i = 0;
	while(i + q - 1 < word_length)
	{
		char* temp_qgram = new char[this->q + 1];
		for(int j = 0; j < q; j++)
		{
			temp_qgram[j] = word[j + i];
		}
		temp_qgram[q] = '\0';
		qgrams.push_back(temp_qgram);
		i++;
	}
	return SUCCESS;
}

int splitWord(const char* line, char c, vector<string>& words)
{
	string str = string(line);
	int len = str.length();
    int start = 0;
    for(int i = 0; i<len; i++)
    {
        if(str[i] == c)
        {
            words.push_back(str.substr(start,i-start));
            while(str[i+1] == c)
            {
                i++;
            }
            start = i+1;
        }
    }
    words.push_back(str.substr(start,len-start));
    sort(words.begin(),words.end());
    words.erase(unique(words.begin(),words.end()),words.end());
    return SUCCESS;
}

int insertEDList(char* qgram, int id)
{
	Qgram temp_q;
	temp_q.qgram = qgram;
	temp_q.index = ed_list.size();
	set<Qgram>::iterator it = ed_set.find(temp_q);
	if(it == ed_set.end())
	{
		ed_list.push_back(vector<int>());
		ed_set.insert(temp_q);
		insertEDList(qgram,id);
	}
	else
	{
		int index = it->index;
		ed_list[index].push_back(id);
	}
	return SUCCESS;
}

int insertJACList(char* item, int id)
{
	Qgram temp_q;
	temp_q.qgram = item;
	temp_q.index = jac_list.size();
	set<Qgram>::iterator it = jac_set.find(temp_q);
	if(it == jac_set.end())
	{
		jac_list.push_back(vector<int>());
		jac_set.insert(temp_q);
		insertJACList(item,id);
	}
	else
	{
		int index = it->index;
		jac_list[index].push_back(id);
	}
	return SUCCESS;
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
	for (int i = 0; i < DIS_SIZE; i++)
		dis[i][0] = i;
	for (int j = 0; j < DIS_SIZE; j++)
		dis[0][j] = j;
	this->q = q;
	int id = 0;
	ifstream fin(filename);	
	while(!fin.eof())
	{
		getline(fin,string_buf);
		if(string_buf.size() == 0)
		{
			break;
		}
		char* buffer = &string_buf[0];
		createList(buffer,id);
		id++;
	}	
	fin.close();
	return SUCCESS;
}

int calculateED(const char *query, const char* entry, int th)
{
	const char* t = query;
	int t_length = strlen(t);
	const char* s = entry;
	int s_length = strlen(s);
	if(abs(s_length - t_length) > th)
		return MAX_NUM;
	if(s_length > t_length)
		return calculateED(entry, query, th);	
	for (int i = 1; i <= s_length; i++)
	{
		int lo = max(1,i-th);
		int hi = min(t_length,i+th);
		bool flag = true;
		for (int j = lo; j <= hi; j++)
		{
			int temp = (s[i - 1] == t[j - 1]) ? 0 : 1;
			if(j == i-th)
			{
				dis[i][j] = min(dis[i - 1][j] + 1, dis[i - 1][j - 1] + temp);
			}
			else if(j == i+th)
			{
				dis[i][j] = min(dis[i][j - 1] + 1, dis[i - 1][j - 1] + temp);
			}
			else
			{
				dis[i][j] = getMin(dis[i - 1][j] + 1, dis[i][j - 1] + 1, dis[i - 1][j - 1] + temp);
			}
			if(dis[i][j] <= th)
				flag = false;
		}
		if(flag)
			return MAX_NUM;
	}
	return dis[s_length][t_length];
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    vector<string> str_words;
    splitWord(query,' ',str_words);

    vector<char*> words;
    for(vector<string>::iterator it = str_words.begin(); it != str_words.end(); it++)
    {
		char* temp_c = new char[it->length() + 10];
		for(unsigned j = 0; j < it->length(); j++)
		{
			temp_c[j] = (*it)[j];
		}
		temp_c[it->length()] = '\0';
		words.push_back(temp_c);
    }
    vector<int> candidate;
    int word_size = words.size();
    int T = max(threshold*word_size, (smin+word_size)*threshold/(threshold+1));
    scanCount(jac_list, jac_set, words, T, candidate);
    sort(candidate.begin(),candidate.end());
    if(T > 0)
    {
        unsigned size = candidate.size();
        for(unsigned i = 0;i < size;i++)
        {
			int id = candidate[i];
			double jaccard = double(candidate_count[id])/double(word_num
				[id]+word_size-candidate_count[id]);
			if(jaccard>=threshold)
				result.push_back(pair<unsigned,double>(id,jaccard));
        }
    }
    else
    {
        for(unsigned i = 0;i < file_lines.size();i++)
        {
			double jaccard = double(candidate_count[i])/double(word_num
				[i]+word_size-candidate_count[i]);
			if(jaccard>=threshold)
				result.push_back(pair<unsigned,double>(i,jaccard));
        }
    }

	return SUCCESS;
}

int SimSearcher::searchED(const char *query, unsigned threshold, vector<pair<unsigned, unsigned> > &result)
{
	result.clear();
	vector<char*> query_words;
	int len = strlen(query);
	for(vector<Line>::iterator it = file_lines.begin(); it != file_lines.end(); ++it)
	{
		char* candidate_word = it->word;
		if(abs(it->length - len) > threshold)
			continue;
		int ed = calculateED(query,candidate_word,threshold);
		if(ed <= (int)threshold)
		{
			result.push_back(make_pair(it->id,ed));
		}
	}
	return SUCCESS;
}



void SimSearcher::printDebug(vector<pair<unsigned, unsigned> > &result)
{
	cout << "ED------------------------\n";

	for(set<Qgram>::iterator it = ed_set.begin();it != ed_set.end();++it)
	{
		int index = it->index;
		vector<int>& list = ed_list[index];
		cout << it->qgram << " : ";
		for(vector<int>::iterator it2 = list.begin();it2 != list.end();++it2)
		{
			cout << *it2 << " , ";
		}
		cout << endl;
	}
	cout << "result:" << endl;
	for(vector<pair<unsigned, unsigned> >::iterator it = result.begin();it != result.end();++it)
	{
		cout << it->first << ":" << file_lines[it->first].word << " ed: " << it->second << endl;
	}

	cout << "JAC-----------------------\n";

	for(set<Qgram>::iterator it = jac_set.begin();it != jac_set.end();++it)
	{
		int index = it->index;
		vector<int>& list = jac_list[index];
		cout << it->qgram << " : ";
		for(vector<int>::iterator it2 = list.begin();it2 != list.end();++it2)
		{
			cout << *it2 << " , ";
		}
		cout << endl;
	}

	cout << "end-------------------------\n";
}
