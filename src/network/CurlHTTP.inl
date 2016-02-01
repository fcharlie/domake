/*
* domake HTTP Library on POSIX
*/
#include <curl/curl.h>
#include <cstring>
#include <algorithm>
#ifndef HTTP_NETWORK_MANAGER_H
#include "HTTPNetworkManager.h"
#endif

struct writeFileStruture {
  FILE *stream;
  size_t size;
  size_t total;
  ProgressCallbackStruture *progress;
  void *Reserved;
};

size_t readHeader(void *data, size_t size, size_t nmemb,
                  writeFileStruture *userdata) {
  if (data && userdata->Reserved) {
    std::string *headers = reinterpret_cast<std::string *>(userdata->Reserved);
    headers->append(reinterpret_cast<char *>(data), size * nmemb);
    auto np = headers->rfind("Content-Length: ");
    auto l = sizeof("Content-Length: ") - 1;
    if (np != std::string::npos) {
      auto np2 = headers->find("\r\n", np);
      if (np2 != std::string::npos) {
        auto clen = headers->substr(np + l, np2 - np - l);
        char *c = nullptr;
        userdata->total = strtol(clen.c_str(), &c, 10);
      }
    }
  }
  return size * nmemb;
}

size_t writeToFile(void *data, size_t size, size_t nmemb,
                   writeFileStruture *userdata) {
  if (userdata == nullptr)
    return 0;
  userdata->size = userdata->size + (size * nmemb);
  // printf("Total: %zu Receive: %zu\n", userdata->total, userdata->size);
  if (userdata->progress && userdata->total > userdata->size) {
    userdata->progress->impl(userdata->size * 100 / userdata->total,
                             userdata->progress->userdata);
  }
  size_t written = fwrite(data, size, nmemb, userdata->stream);
  return written;
}

int DownloadFileSyncInternal(const String &url, const String &path,
                             ProgressCallbackStruture *progress) {
  printf("Path: %s\n", path.c_str());
  CURL *curl;
  FILE *fp;
  CURLcode res;
  std::string npath = path + ".part";
  curl = curl_easy_init();
  if (curl == nullptr) {
    return CURLE_FAILED_INIT;
  }
  fp = fopen(npath.c_str(), "wb");
  if (fp == nullptr) {
    curl_easy_cleanup(curl);
    return 1;
  }
  ////////////////
  std::string headers;
  writeFileStruture wf = {fp, 0, 1, progress, &headers};
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, DEFAULT_USERAGENT);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &wf);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, readHeader);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &wf);
  res = curl_easy_perform(curl);
  switch (res) {
  case CURLE_OK: {
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    fclose(fp);
    if (http_code == 200 || http_code == 201) {
      rename(npath.c_str(), path.c_str());
      if (progress) {
        progress->impl(100, progress->userdata);
      }
    } else {
      remove(npath.c_str());
    }
  } break;
  case CURLE_OPERATION_TIMEDOUT:
  default:
    fclose(fp);
    remove(npath.c_str());
    break;
  }
  /* always cleanup */
  curl_easy_cleanup(curl);
  return res;
}
