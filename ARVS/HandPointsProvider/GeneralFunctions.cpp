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
	//在字符串末尾也加入分隔符，方便截取最后一段
	string strs = str + splits;
	size_t pos = strs.find(splits);
	int step = splits.size();

	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + step, strs.size());
		pos = strs.find(splits);
	}

	return res;
}




