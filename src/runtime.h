/*
*
*/
#ifndef DOMAKE_RUNTIME_H
#define DOMAKE_RUNTIME_H

#include <string>

#ifdef _WIN32
typedef wchar_t Char;
typedef std::wstring String;
#define DEFAULT_USERAGENT L"domake/1.0"
#else
typedef char Char;
typedef std::string String;
#define DEFAULT_USERAGENT "domake/1.0"
#endif

#endif
