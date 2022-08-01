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
//����ļ��Ƿ��ڴ�
GENERALFUNCTIONS_EXPORT bool CheckFileOpened (std::string strFilePath);

GENERALFUNCTIONS_EXPORT void coutMsg(const std::string & msg,bool withEndLine=true);
GENERALFUNCTIONS_EXPORT void coutMsg(const int & msg,bool withEndLine=true);
GENERALFUNCTIONS_EXPORT void coutMsg(const float & msg,bool withEndLine=true);
GENERALFUNCTIONS_EXPORT void coutMsg(const double & msg,bool withEndLine=true);

#endif // GENERALFUNCTIONS_H
