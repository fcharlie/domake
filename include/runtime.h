/*
*
*/
#ifndef DOMAKE_RUNTIME_H
#define DOMAKE_RUNTIME_H

#include <string>

#ifdef _WIN32
typedef wchar_t char_t;
typedef std::wstring string_t;
#define DEFAULT_USERAGENT L"domake/1.0"

#ifndef _X
#define _X(x) __X(x)
#define __X(x) L##x
#endif

#else

#ifndef _X
#define _X(x) __X(x)
#define __X(x) x
#endif

typedef char char_t;
typedef std::string string_t;
#define DEFAULT_USERAGENT "domake/1.0"

#endif

////
class Process {
public:
  Process(const char_t *cmdline);
  int Join();
  int Detch();

private:
  int id;
};

#endif
