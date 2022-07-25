#pragma once
class GeneralFunctions
{
public:
	GeneralFunctions(void);
	~GeneralFunctions(void);
};

#include <vector>
#include <iostream>
#include <string>
#include <io.h>
#include <fstream>
using namespace std;

const double PI=3.141592653;
	
vector<string> SplitString(const string& str, const string& splits);
