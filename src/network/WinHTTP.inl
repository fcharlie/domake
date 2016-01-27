/*
* domake HTTP Library on Windows
*/

#include <windows.h>
/*
	URL_COMPONENTS urlComp;
	DWORD dwUrlLen = 0;
	// Initialize the URL_COMPONENTS structure.
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero 
	// so that they are cracked.
	urlComp.dwSchemeLength = (DWORD)-1;
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;
	urlComp.dwExtraInfoLength = (DWORD)-1;

	if (!WinHttpCrackUrl(url.data(), dwUrlLen, 0, &urlComp)) {
		return 2;
	}
	std::wstring strHostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
	std::wstring strURLPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
*/
//class URL:public 

struct RequestURL {
	int nPort;
	int nScheme;
	std::wstring scheme;
	std::wstring host;
	std::wstring path;
	std::wstring username;
	std::wstring password;
	std::wstring extrainfo;
};

static bool RequestURLResolve(const std::wstring &url, RequestURL &requestUrl) {
	URL_COMPONENTS urlComp;
	DWORD dwUrlLen = 0;
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);
	urlComp.dwSchemeLength = (DWORD)-1;
	urlComp.dwHostNameLength = (DWORD)-1;
	urlComp.dwUrlPathLength = (DWORD)-1;
	urlComp.dwExtraInfoLength = (DWORD)-1;

	if (!WinHttpCrackUrl(url.data(), dwUrlLen, 0, &urlComp)) {
		return false;
	}
	requestUrl.scheme.assign(urlComp.lpszScheme, urlComp.dwSchemeLength);
	requestUrl.host.assign(urlComp.lpszHostName, urlComp.dwHostNameLength);
	requestUrl.path.assign(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
	requestUrl.nPort = urlComp.nPort;
	requestUrl.nScheme = urlComp.nScheme;
	if (urlComp.lpszUserName) {
		requestUrl.username.assign(urlComp.lpszUserName, urlComp.dwUserNameLength);
	}
	if (urlComp.lpszPassword) {
		requestUrl.password.assign(urlComp.lpszPassword, urlComp.dwPasswordLength);
	}
	if (urlComp.lpszExtraInfo) {
		requestUrl.extrainfo.assign(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
	}
	return true;
}
