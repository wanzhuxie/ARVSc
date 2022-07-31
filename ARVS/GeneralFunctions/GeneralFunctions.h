#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

#include "generalfunctions_global.h"
#include <vector>
#include <string>
using namespace std;

class GENERALFUNCTIONS_EXPORT GeneralFunctions
{
public:
	GeneralFunctions();
	~GeneralFunctions();

private:

};

const double PI=3.141592653;

GENERALFUNCTIONS_EXPORT vector<string> SplitString(const string& str, const string& splits);
GENERALFUNCTIONS_EXPORT string GetSysTime_string();
GENERALFUNCTIONS_EXPORT int GetSysTime_number();
GENERALFUNCTIONS_EXPORT std::string IntToStr(int i);
//检查文件是否在打开
GENERALFUNCTIONS_EXPORT bool CheckFileOpened (std::string strFilePath);


#endif // GENERALFUNCTIONS_H
