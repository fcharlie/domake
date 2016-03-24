/*
*
*/
#include "HTTPNetworkManager.h"
string_t ParseURLFile(const string_t &url) {
  auto np = url.rfind(_X('/'));
  string_t file(_X("savefile"));
  if (np != string_t::npos) {
    auto np2 = url.find(_X('?'), np);
    if (np2 == string_t::npos) {
      file = url.substr(np + 1);
    } else {
      file = url.substr(np + 1, np2 - np - 1);
    }
  }
  return file;
}
HTTPResponse::HTTPResponse() {
  ///
}

bool HTTPResponse::ParseHeader(const string_t &header) {
  std::pair<string_t, string_t> pair;
  bool left = false;
  for (auto iter = header.begin(); iter != header.end(); iter++) {
    switch (*iter) {
    case _X(':'):
      left = true;
      if (iter < header.end())
        iter++;
      break;
    case _X('\r'):
      break;
    case _X('\n'):
      if (pair.first.size() && pair.second.size()) {
        header_.insert(pair);
      }
      left = false;
      pair.first.clear();
      pair.second.clear();
      break;
    default:
      if (left) {
        pair.second.push_back(*iter);
      } else {
        pair.first.push_back(*iter);
      }
      break;
    }
  }
  return true;
}

bool HTTPResponse::ParseHeader(char_t *begin, size_t size) {
  if (begin == nullptr || size == 0)
    return false;
  std::pair<string_t, string_t> pair;
  bool left = false;
  auto end = begin + size;
  for (auto iter = begin; iter < end; iter++) {
    switch (*iter) {
    case _X(':'):
      left = true;
      if (iter < end)
        iter++;
      break;
    case _X('\r'):
      break;
    case _X('\n'):
      if (pair.first.size() && pair.second.size()) {
        header_.insert(pair);
      }
      left = false;
      pair.first.clear();
      pair.second.clear();
      break;
    default:
      if (left) {
        pair.second.push_back(*iter);
      } else {
        pair.first.push_back(*iter);
      }
      break;
    }
  }
  return true;
}
