#pragma once

#include <vector>
#include <string>

size_t strLen(const char* str);
size_t strLen(const wchar_t* str);

std::vector<std::string> strSplit(const char* str, char delimiter);
std::vector<std::string> strSplit(const std::string& str, char delimiter);
std::vector<std::wstring> strSplit(const wchar_t* str, wchar_t delimiter);
std::vector<std::wstring> strSplit(const std::wstring& str, wchar_t delimiter);

long long strIndexOf(const char* findIn, const char* findWhat, size_t offset = 0);
long long strIndexOf(const std::string& findIn, const std::string& findWhat, size_t offset = 0);
long long strIndexOf(const wchar_t* findIn, const wchar_t* findWhat, size_t offset = 0);
long long strIndexOf(const std::wstring& findIn, const std::wstring& findWhat, size_t offset = 0);

long long strLastIndexOf(const char* findIn, const char* findWhat, size_t offset = SIZE_MAX);
long long strLastIndexOf(const std::string& findIn, const std::string& findWhat, size_t offset = SIZE_MAX);
long long strLastIndexOf(const wchar_t* findIn, const wchar_t* findWhat, size_t offset = SIZE_MAX);
long long strLastIndexOf(const std::wstring& findIn, const std::wstring& findWhat, size_t offset = SIZE_MAX);

inline bool strContains(const char* container, const char* value);
inline bool strContains(const wchar_t* container, const wchar_t* value);
inline bool strContains(const std::string& container, const std::string& value);
inline bool strContains(const std::wstring& container, const std::wstring& value);

std::string strToAString(const std::string& str);
std::string strToAString(const std::wstring& wstr);
std::wstring strToWString(const std::string& str);
std::wstring strToWString(const std::wstring& wstr);

std::string strToLower(const char* str);
std::string strToUpper(const char* str);
