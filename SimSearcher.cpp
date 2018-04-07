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

int SimSearcher::createList(char* item, int id)
{
	char* new_word = new char[4096];
	strcpy(new_word,item);
	Line e;
	e.word = new_word;
	e.id = id;
	e.length = (int)strlen(new_word);
	file_lines.push_back(e);
	vector<char*> qgrams;
	generateQgrams(item,e.length,qgrams);
	for(vector<char*>::iterator it = qgrams.begin();it != qgrams.end();++it)
		insertEDList(*it,id);
	vector<string> words;
	splitWord(item,' ',words);
	word_num.push_back((int)words.size());
	if (smin >  (int)words.size()) then 
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

int SimSearcher::generateQgrams(const char* word, int word_length, vector<char*>& qgrams)
//produce grams
{
	qgrams.clear();
	int i = 0;
	int len = word_length;
	while(i + q - 1 < len)
	{
		char* qgram_entry = new char[this->q + 1];
		for(int j = 0;j < q;j++)
		{
			qgram_entry[j] = word[j + i];
		}
		qgram_entry[q] = '\0';
		qgrams.push_back(qgram_entry);
		i++;
	}
	return SUCCESS;
}

int SimSearcher::splitWord(const char* line, char c, vector<string>& words)
//split line into single words
{
	string str = string(line);
	int len = str.length();
    int start = 0;
    for(int i = 0;i<len;i++)
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
    //remove the same word
    return SUCCESS;
}

int SimSearcher::insertEDList(char* qgram, int id)
// build inverted list. id begin at the second item
{
	int new_index = ed_list.size();
	Qgram new_qgram;
	new_qgram.qgram = qgram;
	new_qgram.index = new_index;
	set<Qgram>::iterator it = ed_set.find(new_qgram);
	if(it == ed_set.end())
	//do not exist in inverted list
	{
		ed_list.push_back(vector<int>());
		ed_set.insert(new_qgram);
		//add into set
		insertEDList(qgram,id);
		//re-insert into list
	}
	else
	//exist
	{
		int index = it->index;
		ed_list[index].push_back(id);
	}
	return SUCCESS;
}

int SimSearcher::insertJACList(char* item, int id)
{
	int new_index = jac_list.size();
	Qgram new_word;
	new_word.qgram = item;
	new_word.index = new_index;
	set<Qgram>::iterator it = jac_set.find(new_word);
	if(it == jac_set.end())
	{
		jac_list.push_back(vector<int>());
		jac_set.insert(new_word);
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
// create index for further searching
{
	this->q = q;
	ifstream fin(filename);
	int id = 0;
	
	while(!fin.eof())
	{
		getline(fin,string_buf);
		if(string_buf.size() == 0)
		{
			break;
		}
		char* buffer = &string_buf[0];
		createList(buffer,id);
		//call createList
		id++;
	}
	
	fin.close();

	for (int i = 0; i < DIS_SIZE; i++)
		dis[i][0] = i;
	for (int j = 0; j < DIS_SIZE; j++)
		dis[0][j] = j;

	return SUCCESS;
}

int SimSearcher::calED(const char *query, const char* entry, int th)
//calculate ED between query and entry
{
	const char* t = query;
	int lent = strlen(t);
	const char* s = entry;
	int lens = strlen(s);
	if(abs(lens - lent) > th)
		return MAX_NUM;
	if(lens > lent)
		return calED(entry, query, th);
		//when lens > lent swap entry and query to reach the complexity O(min(lens,lent)*(2*th+1))
	
	for (int i = 1; i <= lens; i++)
	{
		int lo = max(1,i-th);
		int hi = min(lent,i+th);
		//lower bound and upper bound
		bool flag = true;
		for (int j = lo; j <= hi; j++)
		//only calculate possible position using the threshold
		{
			int tij = (s[i - 1] == t[j - 1]) ? 0 : 1;
			
			if(j == i-th)
			{
				dis[i][j] = min(dis[i - 1][j] + 1,
	            			   dis[i - 1][j - 1] + tij);
			}
			else if(j == i+th)
			{
				dis[i][j] = min(dis[i][j - 1] + 1,
	            			   dis[i - 1][j - 1] + tij);
			}
			else
			{
				dis[i][j] = getMin(dis[i - 1][j] + 1,
	            			   dis[i][j - 1] + 1,
	            			   dis[i - 1][j - 1] + tij);
			}
			if(dis[i][j] <= th)
				flag = false;
		}
		if(flag)
			return MAX_NUM;
		//early termination
	}
	return dis[lens][lent];
}

int SimSearcher::searchJaccard(const char *query, double threshold, vector<pair<unsigned, double> > &result)
{
	result.clear();
    vector<string> str_words;
    splitWord(query,' ',str_words);
    //split query into words

    vector<char*> words;
    for(vector<string>::iterator it = str_words.begin();it != str_words.end();it++)
    {
		char* tmp = new char[it->length() + 10];
		for(unsigned j = 0;j < it->length();j++)
		{
			tmp[j] = (*it)[j];
		}
		tmp[it->length()] = '\0';
		words.push_back(tmp);
    }
    //translation into char*

    vector<int> candidate;
    int word_size = words.size();
    int T = max(threshold*word_size,(smin+word_size)*threshold/(threshold+1));
    //threshold to choosing candidate
    scanCount(jac_list,jac_set,words,T,candidate);
    sort(candidate.begin(),candidate.end());
    if(T > 0)
    //valid threshold , scan candidate
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
    //else scan all lines
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
	int th = len - q + 1 - q * threshold;
	for(vector<Line>::iterator it = file_lines.begin();it != file_lines.end();++it)
	{
		char* candidate_word = it->word;
		if(abs(it->length - len) > threshold)
			continue;
		int ed = calED(query,candidate_word,threshold);
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
