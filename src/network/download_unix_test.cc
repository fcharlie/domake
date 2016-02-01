//
///
#include <iostream>
#include "HTTPNetworkManager.h"

void ProgressCallback(size_t rate, void *data) { // printf("\r%zu\n", rate);
  if (rate > 100)
    rate = 100;
  std::string s(rate, '#');
  if (rate < 100) {
    printf("\r[%s> %zu%%", s.c_str(), rate);
  } else {
    printf("\r[%s] %zu%%", s.c_str(), rate);
  }
  fflush(stdout);
}

int main() {
  std::string url = "http://npm.taobao.org/mirrors/atom/1.4.1/atom-windows.zip";
  ProgressCallbackStruture progress = {ProgressCallback, nullptr};
  std::cout << "Download File Result: "
            << DownloadFileSync(url, NULL, &progress) << std::endl;
  return 0;
}
