///
///
///
#ifdef _WIN32
#include "HTTPNetworkManager.h"
#include <Shlwapi.h>
#include <unordered_map>
#include <windows.h>
#include <winhttp.h>

static std::unordered_map<int, const wchar_t *> methods = {
    {kHttpVerbGET, L"GET"},         /// GET
    {kHttpVerbPOST, L"POST"},       /// POST
    {kHttpVerbPUT, L"PUT"},         /// PUT
    {kHttpVerbDELETE, L"DELETE"},   // DELETE
    {kHttpVerbHEAD, L"HEAD"},       // HEAD
    {kHttpVerbOPTIONS, L"OPTIONS"}, // OPTIONS
    {kHttpVerbTRACE, L"TRACE"},     // TRACE
    {kHttpVerbCONNECT, L"CONNECT"}  // CONNECT
};

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
    requestUrl.extrainfo.assign(urlComp.lpszExtraInfo,
                                urlComp.dwExtraInfoLength);
  }
  return true;
}

bool ResponseBuilderBody(HTTPResponse &response, const std::string &body) {
  auto &header = response.Header();
  int cp = CP_UTF8;
  auto iter = header.find(L"Content-Type");
  if (iter != header.end()) {
    auto np = iter->second.find(L"charset=");
    if (np != std::wstring::npos) {
      std::wstring charset = iter->second.substr(np);
      if (charset.compare(0, 5, L"utf-8") == 0 ||
          charset.compare(0, 5, L"UTF-8")) {
        cp = CP_UTF8;
      } else if (charset.compare(0, 6, L"GB2312") == 0) {

      } else {
        cp = CP_ACP;
      }
    }
  }
  int len = ::MultiByteToWideChar(cp, 0, (LPCSTR)body.data(), -1, NULL, 0);
  if (len > 0) {
    wchar_t *wstr = new wchar_t[len + 1];
    if (wstr != NULL) {
      len = ::MultiByteToWideChar(cp, 0, (LPCSTR)body.data(), -1, wstr, len);
      if (len > 0) {
        response.ResponseBody().assign(wstr, len);
      }
      delete[] wstr;
    }
    return true;
  }
  return false;
}

int Request(const HTTPRequest &request, HTTPResponse &response) {
  HANDLE hInternet = nullptr;
  HANDLE hConnect = nullptr;
  HANDLE hRequest = nullptr;
  RequestURL zurl;
  DWORD dwHeader = 0;
  DWORD dwSize = 0;
  BOOL bResult = FALSE;
  wchar_t *hbuffer = nullptr;
  int result = kResultOK;
  DWORD dwOption = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
  DWORD dwOpenRequestFlag = 0;
  std::string body;
  BYTE buffer[8192];
  if (!RequestURLResolve(request.URL(), zurl))
    return kErrorArgument;
  if (zurl.nScheme == INTERNET_SCHEME_HTTPS)
    dwOpenRequestFlag = WINHTTP_FLAG_SECURE;
  ///
  hInternet = WinHttpOpen(DEFAULT_USERAGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (hInternet == nullptr)
    return kInitializeProviderFailed;
  if (request.Timeout() != -1) {
    int m_resolveTimeout = request.Timeout();
    int m_connectTimeout = request.Timeout();
    int m_sendTimeout = request.Timeout();
    int m_receiveTimeout = request.Timeout();
    WinHttpSetTimeouts(hInternet, m_resolveTimeout, m_connectTimeout,
                       m_sendTimeout, m_receiveTimeout);
  }
  hConnect = WinHttpConnect(hInternet, zurl.host.c_str(),
                            (INTERNET_PORT)zurl.nPort, 0);
  if (hConnect == nullptr) {
    WinHttpCloseHandle(hInternet);
    return kConnectFailed;
  }

  WinHttpSetOption(hInternet, WINHTTP_OPTION_REDIRECT_POLICY, &dwOption,
                   sizeof(DWORD));

  hRequest = WinHttpOpenRequest(
      hConnect, methods[request.Method()], zurl.path.c_str(), nullptr,
      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
  if (hRequest == nullptr) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hInternet);
    return kProcessingError;
  }
  std::wstring bh;
  // BuilderHeaders(bh,request.Header());
  for (auto &h : request.Header()) {
    bh.assign(h.first);
    bh.append(_X(": "));
    bh.append(h.second);
    bh.append(_X("\r\n"));
    WinHttpAddRequestHeaders(hRequest, bh.data(), bh.size(),
                             WINHTTP_ADDREQ_FLAG_ADD);
  }
  if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                         WINHTTP_NO_REQUEST_DATA, 0, 0, 0) == FALSE) {
    result = kProcessingError;
    goto Cleanup;
  }
  if (request.Method() == kHttpVerbPOST || request.Method() == kHttpVerbPUT) {
    if (!WinHttpWriteData(hRequest, request.RequestBody().c_str(),
                          request.RequestBody().size(), NULL)) {
      result = kProcessingError;
      goto Cleanup;
    }
  }
  if (WinHttpReceiveResponse(hRequest, NULL) == FALSE) {
    result = kProcessingError;
    goto Cleanup;
  }

  bResult = ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                  WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwHeader,
                                  WINHTTP_NO_HEADER_INDEX);
  hbuffer = new wchar_t[dwHeader + 1];
  if (hbuffer) {
    ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                          WINHTTP_HEADER_NAME_BY_INDEX, hbuffer, &dwHeader,
                          WINHTTP_NO_HEADER_INDEX);
    response.ParseHeader(hbuffer, dwHeader);
    delete[] hbuffer;
  } else {
    result = kProcessingError;
    goto Cleanup;
  }

  do {
    // Check for available data.
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
      ///
    }
    DWORD dwDownloaded = 0;
    if (WinHttpReadData(hRequest, (LPVOID)buffer,
                        sizeof(buffer) > dwSize ? dwSize : sizeof(buffer),
                        &dwDownloaded)) {
      body.append((char *)buffer, dwDownloaded);
    }
  } while (dwSize > 0);

  ResponseBuilderBody(response, body);
Cleanup:
  WinHttpCloseHandle(hRequest);
  WinHttpCloseHandle(hConnect);
  WinHttpCloseHandle(hInternet);
  return kResultOK;
}
static bool
ParseHeader(wchar_t *begin, size_t length,
            std::unordered_map<std::wstring, std::wstring> &headers) {
  std::wstring header(begin, length);
  auto np = header.find(L"\r\n");
  size_t offset = 0;
  while (np != std::wstring::npos) {
    auto sp = header.find(L':', offset);
    if (sp != std::wstring::npos && sp < np && sp > offset) {
      headers[header.substr(offset, sp - offset)] =
          header.substr(sp + 2, np - sp - 2);
    }
    offset = np + 2;
    np = header.find(L"\r\n", offset);
  }
  return true;
}

int DownloadFile(const std::wstring &url, const std::wstring &path,
                 ProgressCallbackStruture *progress) {
  RequestURL zurl;
  if (!RequestURLResolve(url, zurl))
    return 1;
  auto hInternet =
      WinHttpOpen(DEFAULT_USERAGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (hInternet == nullptr)
    return 2;
  auto hConnect = WinHttpConnect(hInternet, zurl.host.c_str(),
                                 (INTERNET_PORT)zurl.nPort, 0);
  if (hConnect == nullptr) {
    WinHttpCloseHandle(hInternet);
    return 3;
  }
  DWORD dwOption = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
  WinHttpSetOption(hInternet, WINHTTP_OPTION_REDIRECT_POLICY, &dwOption,
                   sizeof(DWORD));
  DWORD dwOpenRequestFlag =
      (zurl.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
  auto hRequest = WinHttpOpenRequest(
      hConnect, L"GET", zurl.path.c_str(), nullptr, WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
  if (hRequest == nullptr) {
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hInternet);
    return 4;
  }

  if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                         WINHTTP_NO_REQUEST_DATA, 0, 0, 0) == FALSE) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hInternet);
    return 5;
  }

  if (WinHttpReceiveResponse(hRequest, NULL) == FALSE) {
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hInternet);
    return 6;
  }
  DWORD dwHeader = 0;
  BOOL bResult = FALSE;
  bResult = ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                                  WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwHeader,
                                  WINHTTP_NO_HEADER_INDEX);
  auto headerBuffer = new wchar_t[dwHeader + 1];
  size_t contentLength = 0;
  if (headerBuffer) {
    ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                          WINHTTP_HEADER_NAME_BY_INDEX, headerBuffer, &dwHeader,
                          WINHTTP_NO_HEADER_INDEX);
    std::unordered_map<std::wstring, std::wstring> headers;
    ParseHeader(headerBuffer, dwHeader, headers);
    contentLength =
        static_cast<size_t>(_wtoll(headers[L"Content-Length"].c_str()));
    delete[] headerBuffer;
  }
  std::wstring tmp = path + L".part";
  HANDLE hFile =
      CreateFileW(tmp.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
                  NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  DWORD dwSize = 0;
  size_t total = 0;
  if (progress) {
    progress->impl(0, progress->userdata);
  }
  do {
    // Check for available data.
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
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
    if (!pchOutBuffer) {
      dwSize = 0;
    } else {
      DWORD dwDownloaded = 0;
      ZeroMemory(pchOutBuffer, dwSize + 1);
      if (WinHttpReadData(hRequest, (LPVOID)pchOutBuffer, dwSize,
                          &dwDownloaded)) {
        DWORD wmWritten;
        WriteFile(hFile, pchOutBuffer, dwSize, &wmWritten, NULL);
      }
      delete[] pchOutBuffer;
    }
  } while (dwSize > 0);
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
    } else {
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
void WINAPI AsyncCallbackInternel(HINTERNET hInternet, DWORD_PTR dwContext,
                                  DWORD dwInternetStatus,
                                  LPVOID lpvStatusInformation,
                                  DWORD dwStatusInformationLength) {
  /// do something
  (void)hInternet;
  (void)dwContext;
  (void)dwInternetStatus;
  (void)lpvStatusInformation;
  (void)dwStatusInformationLength;
}

#endif
