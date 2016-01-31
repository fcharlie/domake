/*
*
*/
#ifndef HTTP_NETWORK_MANAGER_H
#define HTTP_NETWORK_MANAGER_H

#include <string>
#include <unordered_map>

#ifdef _WIN32
#define Char wchar_t
#define String std::wstring
#define DEFAULT_USERAGENT L"domake/1.0"
#else
#define Char char
#define String std::string
#define DEFAULT_USERAGENT "domake/1.0"
#endif

struct ProgressCallbackStruture {
	void (*impl)( float rate, void *userdata);
    void *userdata;
};


class HTTPRequest{
};

class HTTPResponse{
};

int Request(const HTTPRequest &request,HTTPResponse &response);

int DownloadFileSyncEx(
    const String &url, 
    const String &path,
    ProgressCallbackStruture *progress);

#endif

