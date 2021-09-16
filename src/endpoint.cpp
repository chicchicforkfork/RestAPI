#include "endpoint.h"
#include <cpprest/http_listener.h>
#include <pplx/threadpool.h>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;
using namespace chkchk;

Endpoint::Endpoint(utility::string_t url, size_t listen_thread_num) //
    : _listener(url),                                               //
      _listen_thread_num(listen_thread_num) {
  /// TODO:
  /// https://github.com/microsoft/cpprestsdk/blob/master/Release/src/pplx/threadpool.cpp
  /// initialize_shared_threadpool
  /// std::call_once
  crossplat::threadpool::initialize_with_threads(_listen_thread_num);

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
    jout.update(nlohmann::json::parse(jin.serialize()));
  }
  return jout;
}

static nlohmann::json &appendJSON(nlohmann::json &jout,
                                  const map<string_t, string_t> &map) {
  for (auto m : map) {
    jout[m.first] = m.second;
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

void Endpoint::callapi(API_METHOD method, http_request &message) {
  endpoint_handler_t *endpoint = NULL;
  string_t url = http::uri::decode(message.relative_uri().path());
  string_t resp = "";

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

  if (resp == "") {
    // ucout << "404" << endl;
    message.reply(status_codes::NotFound);
  } else {
    // ucout << resp << endl;
    message.reply(status_codes::OK, resp);
  }
}
