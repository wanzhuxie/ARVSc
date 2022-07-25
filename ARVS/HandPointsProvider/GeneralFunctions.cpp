#include "GeneralFunctions.h"


GeneralFunctions::GeneralFunctions(void)
{
}
GeneralFunctions::~GeneralFunctions(void)
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




