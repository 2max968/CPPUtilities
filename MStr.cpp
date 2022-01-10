#include "MStr.h"
#include <locale>
#include <codecvt>

template <class TChr>
static size_t _strLen(const TChr* str)
{
	size_t i;
	for (i = 0; str[i] != '\0'; i++)
	{
	}
	return i;
}

size_t strLen(const char* str)
{
	return _strLen(str);
}

size_t strLen(const wchar_t* str)
{
	return _strLen(str);
}

template <class TChr>
static std::vector<std::basic_string<TChr>> _strSplit(const TChr* str, TChr delimiter)
{
	std::vector<std::basic_string<TChr>> list;
	size_t ptr = 0;
	for (size_t i = 0; true; i++)
	{
		if (str[i] == delimiter || str[i] == '\0')
		{
			list.push_back(std::basic_string<TChr>(&str[ptr], i - ptr));
			ptr = i + 1;
		}

		if (str[i] == '\0')
			break;
	}
	return list;
}

std::vector<std::string> strSplit(const char* str, char delimiter)
{
	return _strSplit(str, delimiter);
}

std::vector<std::string> strSplit(const std::string& str, char delimiter)
{
	return _strSplit(str.c_str(), delimiter);
}

std::vector<std::wstring> strSplit(const wchar_t* str, wchar_t delimiter)
{
	return _strSplit(str, delimiter);
}

std::vector<std::wstring> strSplit(const std::wstring& str, wchar_t delimiter)
{
	return _strSplit(str.c_str(), delimiter);
}

template <class TChr>
static long _strIndexOf(const TChr* findIn, const TChr* findWhat, size_t offset)
{
	size_t lenWhat = _strLen(findWhat);
	for (size_t i = offset; findIn[i + lenWhat - 1] != '\0'; i++)
	{
		bool diff = false;
		for (size_t j = 0; j < lenWhat; j++)
		{
			if (findIn[i + j] != findWhat[j])
			{
				diff = true;
				break;
			}
		}
		if (!diff)
			return i;
	}
	return -1;
}

long long strIndexOf(const char* findIn, const char* findWhat, size_t offset)
{
	return _strIndexOf(findIn, findWhat, offset);
}

long long strIndexOf(const std::string& findIn, const std::string& findWhat, size_t offset)
{
	return _strIndexOf(findIn.c_str(), findWhat.c_str(), offset);
}

long long strIndexOf(const wchar_t* findIn, const wchar_t* findWhat, size_t offset)
{
	return _strIndexOf(findIn, findWhat, offset);
}

long long strIndexOf(const std::wstring& findIn, const std::wstring& findWhat, size_t offset)
{
	return _strIndexOf(findIn.c_str(), findWhat.c_str(), offset);
}

template <class TChr>
static long _strLastIndexOf(const TChr* findIn, const TChr* findWhat, size_t offset)
{
	size_t lenIn = _strLen(findIn);
	size_t lenWhat = _strLen(findWhat);
	size_t start = lenIn - lenWhat;
	if (offset < start) start = offset;
	for (size_t i = start; true; i--)
	{
		bool diff = false;
		for (size_t j = 0; j < lenWhat; j++)
		{
			if (findIn[i + j] != findWhat[j])
			{
				diff = true;
				break;
			}
		}
		if (!diff)
			return i;

		if (i == 0)
			break;
	}
	return -1;
}

long long strLastIndexOf(const char* findIn, const char* findWhat, size_t offset)
{
	return _strLastIndexOf(findIn, findWhat, offset);
}

long long strLastIndexOf(const std::string& findIn, const std::string& findWhat, size_t offset)
{
	return _strLastIndexOf(findIn.c_str(), findWhat.c_str(), offset);
}

long long strLastIndexOf(const wchar_t* findIn, const wchar_t* findWhat, size_t offset)
{
	return _strLastIndexOf(findIn, findWhat, offset);
}

long long strLastIndexOf(const std::wstring& findIn, const std::wstring& findWhat, size_t offset)
{
	return _strLastIndexOf(findIn.c_str(), findWhat.c_str(), offset);
}

std::string strToAString(const std::string& str)
{
	return str;
}

std::string strToAString(const std::wstring& wstr)
{
	using conv_t = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<conv_t, wchar_t> conv;

	return conv.to_bytes(wstr);
}

std::wstring strToWString(const std::string& str)
{
	using conv_t = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<conv_t, wchar_t> conv;

	return conv.from_bytes(str);
}

std::wstring strToWString(const std::wstring& wstr)
{
	return wstr;
}

inline bool strContains(const char* container, const char* value)
{
	return strIndexOf(container, value) > 0;
}

inline bool strContains(const wchar_t* container, const wchar_t* value)
{
	return strIndexOf(container, value) > 0;
}

inline bool strContains(const std::string& container, const std::string& value)
{
	return strIndexOf(container, value) > 0;
}

inline bool strContains(const std::wstring& container, const std::wstring& value)
{
	return strIndexOf(container, value) > 0;
}

std::string strToLower(const char* str)
{
	size_t len = _strLen(str);
	std::string ret = std::string(len, '+');

	for (int i = 0; i < len; i++)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
		{
			ret[i] = str[i] | 32;
		}
		else
		{
			ret[i] = str[i];
		}
	}

	return ret;
}

std::string strToUpper(const char* str)
{
	size_t len = _strLen(str);
	std::string ret = std::string(len, '+');

	for (int i = 0; i < len; i++)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
		{
			ret[i] = str[i] & ~32;
		}
		else
		{
			ret[i] = str[i];
		}
	}

	return ret;
}
