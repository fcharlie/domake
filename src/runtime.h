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

#ifndef _T
#define _T(x) __T(x)
#define __T(x) L##x
#endif

#else

#ifndef _T
#define _T(x) __T(x)
#define __T(x) L##x
#endif

typedef char Char;
typedef std::string String;
#define DEFAULT_USERAGENT "domake/1.0"

#endif

////
class Process{
public:
    Process(const Char *cmdline);
    int Join();
    int Detch();
private:
    int id;
};


#endif
