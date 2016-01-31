/*
* domake HTTP Library on Windows
*/

#include <windows.h>
#include <winhttp.h>
#include <Shlwapi.h>
#include <unordered_map>
#ifndef HTTP_NETWORK_MANAGER_H
#include "HTTPNetworkManager.h"
#endif

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

static bool ParseHeader(
    wchar_t *begin,
    size_t length,
    std::unordered_map<std::wstring, std::wstring> &headers)
{
	std::wstring header(begin, length);
	auto np = header.find(L"\r\n");
	size_t offset = 0;
	while (np != std::wstring::npos) {
		auto sp = header.find(L':',offset);
		if (sp != std::wstring::npos && sp<np && sp>offset) {
			headers[header.substr(offset, sp-offset)] = header.substr(sp + 2,np-sp-2);
		}
		offset = np + 2;
		np = header.find(L"\r\n", offset);
	}
	return true;
}

/*
void ProgressUpdate(size_t rate, void *data) {
	std::string sp(rate,'#');
	if (rate == 100) {
		printf("\rDownload: [%s] %d%%", sp.c_str(), rate);
	}else {
		printf("\rDownload: [%s> %d%%", sp.c_str(), rate);
	}
}
*/

int DownloadFileSyncEx(const std::wstring &url, const std::wstring &path,ProgressCallbackStruture *progress)
{
	RequestURL zurl;
	if (!RequestURLResolve(url, zurl)) return 1;
	auto hInternet = WinHttpOpen(
        DEFAULT_USERAGENT,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);
	if (hInternet == nullptr) return 2;
	auto hConnect = WinHttpConnect(hInternet, zurl.host.c_str(),(INTERNET_PORT)zurl.nPort, 0);
	if (hConnect == nullptr) {
		WinHttpCloseHandle(hInternet);
		return 3;
	}
	DWORD dwOption = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
	WinHttpSetOption(hInternet, WINHTTP_OPTION_REDIRECT_POLICY, &dwOption, sizeof(DWORD));
	DWORD dwOpenRequestFlag = (zurl.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	auto hRequest = WinHttpOpenRequest(hConnect, L"GET", zurl.path.c_str(),
		nullptr, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
	if (hRequest == nullptr) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hInternet);
		return 4;
	}

	if (WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0,
		0, 0) == FALSE)
	{
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hInternet);
		return 5;
	}

	if (WinHttpReceiveResponse(hRequest, NULL) == FALSE)
	{
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hInternet);
		return 6;
	}
	DWORD dwHeader = 0;
	BOOL bResult = FALSE;
	bResult = ::WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX,
		NULL,
		&dwHeader,
		WINHTTP_NO_HEADER_INDEX);
	auto headerBuffer = new wchar_t[dwHeader + 1];
	size_t contentLength = 0;
	if (headerBuffer) {
		::WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			headerBuffer,
			&dwHeader,
			WINHTTP_NO_HEADER_INDEX);
		std::unordered_map<std::wstring, std::wstring> headers;
		ParseHeader(headerBuffer, dwHeader, headers);
		contentLength = static_cast<size_t>(_wtoll(headers[L"Content-Length"].c_str()));
		delete[] headerBuffer;
	}
	std::wstring tmp = path + L".part";
	HANDLE hFile = CreateFileW(
		tmp.c_str(),
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	DWORD dwSize = 0;
	size_t total = 0;
	if (progress) {
		progress->impl(0, progress->userdata);
	}
	do{
		// Check for available data.
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)){
			/// 
		}
		total += dwSize;
		if (contentLength > 0) {
			if (progress) {
				progress->impl(total * 100 / contentLength, progress->userdata);
			}
		}
		// Allocate space for the buffer.
		BYTE *pchOutBuffer = new BYTE[dwSize + 1];
		if (!pchOutBuffer){
			dwSize = 0;
		}
		else {
			DWORD dwDownloaded = 0;
			ZeroMemory(pchOutBuffer, dwSize + 1);
			if (WinHttpReadData(hRequest, (LPVOID)pchOutBuffer, dwSize, &dwDownloaded)) {
				DWORD wmWritten;
				WriteFile(hFile, pchOutBuffer, dwSize, &wmWritten, NULL);
			}
			delete[] pchOutBuffer;
		}
	}while (dwSize>0);
	if (progress) {
		progress->impl(100, progress->userdata);
	}
	std::wstring npath = path;
	int i = 1;
	while (PathFileExistsW(npath.c_str())) {
		auto n = path.find_last_of('.');
		if (n != std::wstring::npos) {
			npath = path.substr(0, n) + L"(";
			npath += std::to_wstring(i);
			npath += L")";
			npath += path.substr(n);
		}
		else {
			npath = path + L"(" + std::to_wstring(i) + L")";
		}
		i++;
	}
	CloseHandle(hFile);
	MoveFileExW(tmp.c_str(), npath.c_str(), MOVEFILE_COPY_ALLOWED);
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hInternet);
    return 0;
}

void WINAPI AsyncCallbackInternel(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength){
        /// do something
        (void)hInternet;
        (void)dwContext;
        (void)dwInternetStatus;
        (void)lpvStatusInformation;
        (void)dwStatusInformationLength;
}
