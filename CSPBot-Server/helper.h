#pragma once
#include <string>
#include "websocket.h"
#include <Nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

namespace Helper {
	inline bool IsJsonData(std::string strData)
	{
		if (strData[0] != '{')
			return false;

		int num = 1;
		for (int i = 1; i < strData.length(); ++i)
		{
			if (strData[i] == '{')
			{
				++num;
			}
			else if (strData[i] == '}')
			{
				--num;
			}

			if (num == 0)
			{
				return true;
			}
		}

		return false;
	}
	inline webJson transJson(json j) {
		webJson wj;
		wj.packet = j["packet"].get<std::string>();
		wj.data = j["data"].get<std::string>();
		return wj;
	}

	//×Ö·û´®Ìæ»»
	inline std::string replace(std::string strSrc,
		const std::string& oldStr, const std::string& newStr, int count = -1)
	{
		std::string strRet = strSrc;
		size_t pos = 0;
		int l_count = 0;
		if (-1 == count) // replace all
			count = strRet.size();
		while ((pos = strRet.find(oldStr, pos)) != std::string::npos)
		{
			strRet.replace(pos, oldStr.size(), newStr);
			if (++l_count >= count) break;
			pos += newStr.size();
		}
		return strRet;
	}
}