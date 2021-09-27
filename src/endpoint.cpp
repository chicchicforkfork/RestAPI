#include "endpoint.h"
#include <cpprest/http_listener.h>
#include <pplx/threadpool.h>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
using namespace chkchk;

Endpoint::Endpoint(utility::string_t url) : _listener(url) {

  _listener.support(methods::GET, //
                    bind(&Endpoint::get, this, placeholders::_1));
  _listener.support(methods::PUT, //
                    bind(&Endpoint::put, this, placeholders::_1));
  _listener.support(methods::POST, //
                    bind(&Endpoint::post, this, placeholders::_1));
  _listener.support(methods::DEL, //
                    bind(&Endpoint::del, this, placeholders::_1));
  _listener.support(methods::OPTIONS, //
                    bind(&Endpoint::options, this, placeholders::_1));
}

static nlohmann::json &appendJSON(nlohmann::json &jout,
                                  const json::value &jin) {
  if (!jin.is_null()) {
    string_t decode = http::uri::decode(jin.serialize());
    jout.update(nlohmann::json::parse(decode));
  }
  return jout;
}

static nlohmann::json &appendJSON(nlohmann::json &jout,
                                  const map<string_t, string_t> &map) {
  for (auto m : map) {
    string_t k_decode = http::uri::decode(m.first);
    string_t d_decode = http::uri::decode(m.second);
    jout[k_decode] = d_decode;
  }
  return jout;
}

bool Endpoint::addEndpoint(API_METHOD method,            //
                           const utility::string_t path, //
                           endpoint_handler_t handle) {
  auto &endpoint = _tables[method];
  if (endpoint.find(path) != endpoint.end()) {
    return false;
  }
  endpoint[path] = handle;
  return true;
}

endpoint_handler_t *Endpoint::getEndpoint(API_METHOD method, //
                                          const utility::string_t path) {
  auto &endpoint = _tables[method];
  if (endpoint.find(path) == endpoint.end()) {
    return nullptr;
  }
  return &endpoint[path];
}

void Endpoint::post(http_request message) {
  return callapi(API_POST, message);
};

void Endpoint::del(http_request message) { //
  return callapi(API_DEL, message);
};

void Endpoint::put(http_request message) { //
  return callapi(API_PUT, message);
};

void Endpoint::get(http_request message) { //
  return callapi(API_GET, message);
}

void Endpoint::options(http_request message) { //
  return callapi(API_OPTIONS, message);
}

const string method_name(API_METHOD method) {
  static vector<string> ms{"GET", "PUT", "POST", "DELETE", "OPTIONS"};
  return ms[method];
}

API_METHOD method_name(const string method) {
  if (method == "GET") {
    return API_METHOD::API_GET;
  } else if (method == "PUT") {
    return API_METHOD::API_PUT;
  } else if (method == "POST") {
    return API_METHOD::API_POST;
  } else if (method == "DELETE") {
    return API_METHOD::API_DEL;
  } else if (method == "OPTIONS") {
    return API_METHOD::API_OPTIONS;
  }
  return API_METHOD::API_UNKNOWN;
}

void Endpoint::callapi(API_METHOD method, http_request &message) {
  endpoint_handler_t *endpoint = NULL;
  string_t url = http::uri::decode(message.relative_uri().path());
  string_t resp = "";

  if (method == API_METHOD::API_OPTIONS) {
    auto origin_method =
        message.headers().find("Access-Control-Request-Method");
    if (origin_method != message.headers().end()) {
      // cout << (*origin_method).first << "  " << (*origin_method).second <<
      // endl;
      method = method_name((*origin_method).second);
    }
  }

  endpoint = getEndpoint(method, url);
  if (endpoint) {
    nlohmann::json params;
    try {
      const auto &p = message.extract_json().get();
      params = appendJSON(params, p);
    } catch (http_exception &e) {
      // cerr << "[extract_json] " << e.what() << "\n";
    }
    try {
      const auto &p = http::uri::split_query(message.extract_string().get());
      params = appendJSON(params, p);
    } catch (http_exception &e) {
      // cerr << "[extract_string] " << e.what() << "\n";
    }
    try {
      const auto &p = http::uri::split_query(message.relative_uri().query());
      params = appendJSON(params, p);
    } catch (http_exception &e) {
      // cerr << "[relative_uri] " << e.what() << "\n";
    }
    resp = (*endpoint)(message, params);
  }

  http_response r;
  if (resp == "") {
    r.set_status_code(status_codes::NotFound);
  } else {
    r.set_body(resp);
    r.headers().add("Access-Control-Allow-Origin", "*");
    const char *all_method = "GET,PUT,POST,DELETE,PATCH,OPTIONS";
    r.headers().add("Access-Control-Allow-Methods", all_method);
    r.headers().add("Access-Control-Allow-Headers", "application/json");
    r.set_status_code(status_codes::OK);
  }
  message.reply(r);
}
