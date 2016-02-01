/*
*
*/
#include "HTTPNetworkManager.h"

inline String ParseURLFile(const String &url) {
  auto np = url.rfind('/');
  String file(_T("savefile"));
  if (np != String::npos) {
    auto np2 = url.find('?', np);
    if (np2 == String::npos) {
      file = url.substr(np + 1);
    } else {
      file = url.substr(np + 1, np2 - np - 1);
    }
  }
  return file;
}

#ifdef _WIN32
#include "WinHTTP.inl"
#else
#include "CurlHTTP.inl"
#endif

int DownloadFileSync(const String &url, const Char *saveFile,
                     ProgressCallbackStruture *progress) {
  if (url.empty())
    return 1;
  String file;
  if (saveFile) {
    file = saveFile;
  } else {
    file = ParseURLFile(url);
  }
  return DownloadFileSyncInternal(url, file, progress);
}
