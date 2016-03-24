////

#ifndef _WIN32
#include <algorithm>
#include <cstring>
#include <curl/curl.h>
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

int DownloadFile(const std::string &url, const std::string &path,
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

class UploadFile {
public:
  UploadFile() {}
  ~UploadFile() {
    if (fp) {
      fclose(fp);
    }
  }
  bool Open(const std::string &path) {
    fp = fopen(path.data(), "rb");
    return (fp != nullptr);
  }
  FILE *Get() { return fp; }

private:
  FILE *fp = nullptr;
};

static std::unordered_map<int, const char *> methods = {
    {kHttpVerbGET, "GET"},         /// GET
    {kHttpVerbPOST, "POST"},       /// POST
    {kHttpVerbPUT, "PUT"},         /// PUT
    {kHttpVerbDELETE, "DELETE"},   // DELETE
    {kHttpVerbHEAD, "HEAD"},       // HEAD
    {kHttpVerbOPTIONS, "OPTIONS"}, // OPTIONS
    {kHttpVerbTRACE, "TRACE"},     // TRACE
    {kHttpVerbCONNECT, "CONNECT"}  // CONNECT
};
///
static size_t write_callback(void *data, size_t size, size_t nmemb,
                             void *userdata) {
  HTTPResponse *r;
  r = reinterpret_cast<HTTPResponse *>(userdata);
  r->ResponseBody().append(reinterpret_cast<char *>(data), size * nmemb);
  return (size * nmemb);
}

// header callback function
static size_t header_callback(void *data, size_t size, size_t nmemb,
                              void *userdata) {
  auto rh = reinterpret_cast<std::string *>(userdata);
  rh->append(reinterpret_cast<char *>(data), size * nmemb);
  return (size * nmemb);
}

// read callback function
static size_t read_callback(void *data, size_t size, size_t nmemb,
                            void *userdata) {
  UploadFile *obj = reinterpret_cast<UploadFile *>(userdata);
  size_t sz = size * nmemb;
  fread(data, size, nmemb, obj->Get());
  return sz;
}

class StaticInitialize {
public:
  StaticInitialize() {
    //
    curl_global_init(CURL_GLOBAL_ALL);
  }
  ~StaticInitialize() {
    curl_global_cleanup();
    ///
  }
};

int Request(const HTTPRequest &request, HTTPResponse &response) {
  static StaticInitialize si;
  int result = kResultOK;
  CURL *curl = nullptr;
  CURLcode res = CURLE_OK;
  UploadFile obj;
  if (request.URL().empty())
    return kErrorArgument;
  if ((curl = curl_easy_init()) == nullptr)
    return kInitializeProviderFailed;
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  if (!request.AuthorizationToken().empty()) {
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD,
                     request.AuthorizationToken().data());
  }
  if (request.UserAgent().empty()) {
    curl_easy_setopt(curl, CURLOPT_USERAGENT, DEFAULT_USERAGENT);
  } else {
    curl_easy_setopt(curl, CURLOPT_USERAGENT, request.UserAgent().data());
  }
  curl_easy_setopt(curl, CURLOPT_URL, request.URL().data());
  std::string requestHeader;
  curl_slist *hlist = NULL;
  for (auto &h : request.Header()) {
    requestHeader.assign(h.first);
    requestHeader.append(": ");
    requestHeader.append(h.second);
    hlist = curl_slist_append(hlist, requestHeader.c_str());
  }
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);
  /** perform the actual query */
  if (request.Timeout() != -1) {
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, request.Timeout());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    // dont want to get a sig alarm on timeout
  }
  switch (request.Method()) {
  case kHttpVerbGET:
    break;
  case kHttpVerbPOST:
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.RequestBody().data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.RequestBody().size());
    break;
  case kHttpVerbPUT:
    if (request.PutFile().empty())
      return kPutFileError;
    if (!obj.Open(request.PutFile()))
      return kPutFileError;
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &obj);
    break;
  case kHttpVerbDELETE:
  case kHttpVerbHEAD:
  case kHttpVerbOPTIONS:
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, methods[request.Method()]);
    break;
  default:
    break;
  }
  std::string responseHeader;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeader);

  res = curl_easy_perform(curl);
  switch (res) {
  case CURLE_OK: {
    long http_code = 0;
    response.ParseHeader(responseHeader);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    response.StatusCode() = static_cast<int>(http_code);
  } break;
  case CURLE_OPERATION_TIMEDOUT: {
    response.StatusCode() = 408;
    response.ResponseBody() = "Operation Timeout.";
  } break;
  default: {
    response.StatusCode() = res;
    response.ResponseBody() = curl_easy_strerror(res);
    result = kProcessingError;
  } break;
  }
  curl_slist_free_all(hlist);
  curl_easy_cleanup(curl);
  return result;
}

#endif
