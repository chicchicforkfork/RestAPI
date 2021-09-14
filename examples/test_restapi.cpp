#include "restapi.h"
#include <iostream>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace chkchk;

int main() {
  RestApi api("http://127.0.0.1:9999", "test");
  auto f = [](const http::http_request &r, nlohmann::json &v) {
    (void)r;
    cout << "v: " << v << endl;

    if (!v.is_null()) {
      return v.dump();
    }
    nlohmann::json j;
    j["result"] = "no message";
    return j.dump();
  };

  api.addAPI(API_GET, "/echo/name", f);
  api.addAPI(API_POST, "/echo/name", f);
  api.on_initialize();
  getchar();
  api.on_shutdown();
}
