#pragma once
#include <vector>
#include <string>
using namespace std;


class GeneralFunctions
{
public:
	GeneralFunctions(void);
	~GeneralFunctions(void);
};


const double PI=3.141592653;
	
vector<string> SplitString(const string& str, const string& splits);
string GetSysTime_string();
int GetSysTime_number();
std::string IntToStr(int i);


