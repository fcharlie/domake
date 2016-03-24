/*
*
*/
#ifndef HTTP_NETWORK_MANAGER_H
#define HTTP_NETWORK_MANAGER_H

#include "runtime.h"
#include <unordered_map>

#ifdef _WIN32
#define DEFAULT_USERAGENT L"domake/1.0"
#else
#define DEFAULT_USERAGENT "domake/1.0"
#endif

struct ProgressCallbackStruture {
  void (*impl)(size_t rate, void *userdata);
  void *userdata;
};

enum HTTPRequestMethods {
  kHttpVerbGET,
  kHttpVerbPOST,
  kHttpVerbPUT,
  kHttpVerbDELETE,
  kHttpVerbHEAD,
  kHttpVerbOPTIONS,
  kHttpVerbTRACE,
  kHttpVerbCONNECT
};
typedef std::unordered_map<string_t, string_t> Headers;

enum HTTPResultCodes {
  kResultOK = 0,
  kErrorArgument,
  kInitializeProviderFailed,
  kConnectFailed,
  kPutFileError,
  kProcessingError,
  kProviderUnknownError,
};

class HTTPRequest {
public:
  HTTPRequest() : url_(), method_(kHttpVerbGET), timeout_(-1) {
    /// empty
  }
  HTTPRequest(const char_t *url, int method = kHttpVerbGET, int timeout = -1)
      : url_(url ? url : _X("")), method_(method), timeout_(timeout) {
    /// url empty
  }
  HTTPRequest(const string_t &url, int method = kHttpVerbGET, int timeout = -1)
      : url_(url), method_(method), timeout_(timeout) {}
  const Headers &Header() const { return headers_; }
  Headers &Header() { return headers_; }
  const string_t &URL() const { return url_; }
  string_t &URL() { return url_; }
  const string_t &UserAgent() const { return ua_; }
  string_t &UserAgent() { return ua_; }
  const string_t &ContentType() const { return ctype_; }
  string_t &ContentType() { return ctype_; }
  const string_t &AuthorizationToken() const { return authtoken_; }
  bool SetAuthorizationToken(const char_t *username, const char_t *pwd) {
    if (username == nullptr || pwd == nullptr)
      return false;
    authtoken_.assign(username);
    authtoken_.push_back(_X(':'));
    authtoken_.append(pwd);
    return true;
  }
  const string_t &RequestBody() const { return body_; }
  string_t &RequestBody() { return body_; }
  const string_t &PutFile() const { return file_; }
  string_t &PutFile() { return file_; }
  int Method() const { return method_; }
  int &Method() { return method_; }
  int Timeout() const { return timeout_; }
  int &Timeout() { return timeout_; }

private:
  string_t url_;
  int method_;
  int timeout_;
  Headers headers_;
  string_t ua_;
  string_t ctype_;
  string_t authtoken_;
  string_t body_;
  string_t file_;
};

enum HTTPStatusCodes {
  // 1xx Informational
  kHTTPContinue = 100,   // Continue
  kHTTPSwitch = 101,     // Switching Protocols
  kHTTPProcessing = 102, // WebDAV
  // 2xx Success
  kHTTPOK = 200,
  kHTTPCreated = 201,
  kHTTPAccepted = 202,
  kHTTPNonAuthoritativeInformation = 203,
  kHTTPNoContent = 204,
  kHTTPResetContent = 205,
  kHTTPPartialContent = 206,
  kHTTPMultiStatus = 207,     // WebDAV
  kHTTPAlreadyReported = 208, // WebDAV
  kHTTPIMUsed = 226,
  // 3xx Redirection
  kHTTPMutipleChoices = 300,
  kHTTPMovedPermanently = 301,
  kHTTPFound = 302,
  kHTTPSeeOther = 303,
  kHTTPNotModified = 304,
  kHTTPUseProxy = 305,
  kHTTPUnused = 306,
  kHTTPTemporaryRedirect = 307,
  kHTTPPermanentRedirect = 308, // experiemental
  // 4xx Client Error
  kHTTPBadRequest = 400,
  kHTTPUnauthorized = 401,
  kHTTPPaymentRequired = 402,
  kHTTPForbidden = 403,
  kHTTPNotFound = 404,
  kHTTPMethodNotAllowed = 405,
  kHTTPNotAcceptable = 406,
  kHTTPProxtAuthenticationRequired = 407,
  kHTTPRequestTimeout = 408,
  kHTTPConflict = 409,
  kHTTPGone = 410,
  kHTTPLengthRequired = 411,
  kHTTPPreconditionFailed = 412,
  kHTTPRequestEntityTooLagre = 413,
  kHTTPRequestURITooLong = 414,
  kHTTPUnsupportedMediaType = 415,
  kHTTPRequestedRangeNotSatisfiable = 416,
  kHTTPExpectationFailed = 417,
  kHTTPIMaTeadpot = 418,          // RFC 2324
  kHTTPEnhanceYouCalm = 420,      // Twitter
  kHTTPUnprocessableEntity = 422, // WebDAV
  kHTTPLocked = 423,              // WebDAV
  kHTTPFailedDependency = 424,    // WebDAV
  kHTTPReservedForWebDAV = 425,
  kHTTPUpgradeRequired = 426,
  kHTTPPreconditionRequired = 428,
  kHTTPTooManyRequests = 429,
  kHTTPRequestHeadrFieldsTooLarge = 431,
  kHTTPNoResponse = 444,                     // Nginx
  kHTTPRetryWith = 449,                      // Microsoft
  kHTTPBlockByWindowsParentalControls = 450, // Microsoft
  kHTTPUnavailableForLegalReasons = 451,
  kHTTPClientClisedRequest = 499, // Nginx
  // 5xx Server Error
  kHTTPInternalServerError = 500,
  kHTTPNotImplemented = 501,
  kHTTPBadGateway = 502,
  kHTTPServiceUnavailable = 503,
  kHTTPGatewayTimeout = 504,
  kHTTPVersionNotSupported = 505,
  kHTTPVariantAsloNegotiates = 506,  // Experimental
  kHTTPInsufficientStorage = 507,    // WebDAV
  kHTTPLoopDetected = 508,           // WebDAV
  kHTTPBandwidthLimitExceeded = 509, // Apache
  kHTTPNotExtended = 510,
  kHTTPNetworkAuthenticationRequired = 511,
  kHTTPNetworkReadTimeoutError = 598,
  kHTTPNetworkConnectTimeoutError = 599
};

class HTTPResponse {
public:
  HTTPResponse();
  bool ParseHeader(const string_t &header);
  bool ParseHeader(char_t *begin, size_t size);
  int StatusCode() const { return statuscode; }
  int &StatusCode() { return statuscode; }
  const string_t &ResponseBody() const { return body_; }
  string_t &ResponseBody() { return body_; }
  const Headers &Header() const { return header_; }

private:
  int statuscode;
  Headers header_;
  string_t body_;
};

string_t ParseURLFile(const string_t &url);
int Request(const HTTPRequest &request, HTTPResponse &response);
int DownloadFile(const string_t &url, const char_t *saveFile,
                 ProgressCallbackStruture *progress);

#endif
