#include "GeneralFunctions.h"
#include <windows.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <io.h>

GeneralFunctions::GeneralFunctions()
{

}

GeneralFunctions::~GeneralFunctions()
{

}



// SplitString
vector<string> SplitString(const string& str, const string& splits)
{
	vector<string> res;
	if (str == "")		return res;
	//���ַ���ĩβҲ����ָ����������ȡ���һ��
	string strs = str + splits;
	size_t pos = strs.find(splits);
	int step = splits.size();

	// ���Ҳ����������ַ��������������� npos
	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
		strs = strs.substr(pos + step, strs.size());
		pos = strs.find(splits);
	}

	return res;
}

string GetSysTime_string()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	std::stringstream ss;

	ss<<st.wYear;
	ss<<st.wMonth;
	ss<<st.wDay;
	ss<<st.wHour;
	ss<<st.wMinute;
	ss<<st.wSecond;
	ss<<st.wMilliseconds;

	string strTime=	ss.str();
	//cout<<strTime<<endl;

	return strTime;
}

int GetSysTime_number()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	int iTime=st.wHour*60*60*1000+st.wMinute*60*1000+st.wSecond*1000+st.wMilliseconds;
	return iTime;
}

std::string IntToStr(int i)
{
	std::stringstream ss;
	ss<<i;
	return ss.str();
}

//����ļ��Ƿ��ڴ�
bool CheckFileOpened (std::string strFilePath)
{
	HANDLE hFile = CreateFileA (strFilePath.c_str() , GENERIC_READ , 0 , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle (hFile);
		return TRUE;
	}
	CloseHandle (hFile);
	return FALSE;
}

void coutMsg(const std::string & msg,bool withEndLine)
{
	cout<<msg;

	if(withEndLine)
		cout<<endl;
}
void coutMsg(const int & msg,bool withEndLine)
{
	cout<<msg<<endl;

	if(withEndLine)
		cout<<endl;
}
void coutMsg(const float & msg,bool withEndLine)
{
	cout<<msg<<endl;

	if(withEndLine)
		cout<<endl;
}
void coutMsg(const double & msg,bool withEndLine)
{
	cout<<msg<<endl;

	if(withEndLine)
		cout<<endl;
}


