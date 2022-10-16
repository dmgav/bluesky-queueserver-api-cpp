#ifndef INCLUDE_QSERVER_UTILS_
#define INCLUDE_QSERVER_UTILS_

#include <json.hpp>
#include <string>

namespace qserver {

struct RequestParams {
  using json = nlohmann::json;

  json params;
  httplib::Headers headers;
  httplib::MultipartFormDataItems form_data;

  RequestParams(){};
  RequestParams(const json &params_) : params(params_){};
  RequestParams(json &&params_) : params(std::move(params_)){};
  RequestParams(const json &params_, const httplib::Headers &headers_) : params(params_), headers(headers_){};
  RequestParams(const json &params_, httplib::Headers &&headers_)
      : params(params_), headers(std::move(headers_)){};
  RequestParams(json &&params_, const httplib::Headers &headers_)
      : params(std::move(params_)), headers(headers_){};
  RequestParams(json &&params_, httplib::Headers &&headers_)
      : params(std::move(params_)), headers(std::move(headers_)){};
  RequestParams(const httplib::MultipartFormDataItems &form_data_) : form_data(form_data_){};
  RequestParams(httplib::MultipartFormDataItems &&form_data_) : form_data(std::move(form_data_)){};

  RequestParams(const RequestParams &) = default;
  RequestParams(RequestParams &&) = default;
  RequestParams &operator=(const RequestParams &) = default;
  RequestParams &operator=(RequestParams &&) = default;
  ~RequestParams(){};
};

enum Protocol { PROTOCOL_NONE, PROTOCOL_ZMQ, PROTOCOL_HTTP };

enum HTTPMethod { HTTP_GET, HTTP_POST, HTTP_DELETE };

enum SecurityType { NO_KEY, PUBLIC_KEY, API_KEY, TOKEN };

struct Method {
  HTTPMethod http_method;
  std::string path;

  Method(HTTPMethod _http_method, const std::string &_path) {
    http_method = _http_method;
    path = _path;
  }
};

struct RequestResult {
  bool success;
  std::string msg;
  json payload;

  std::string to_string() {
    std::stringstream ss;
    ss << "{\"success\":" << success << ", \"msg\": \"" << msg << "\", \"payload\": " << payload << "}";
    return ss.str();
  }
};

inline std::ostream &operator<<(std::ostream &os, RequestResult &rr) {
  os << rr.to_string();
  return os;
}

class CommError : public std::exception {
  std::string msg = "";

public:
  virtual const char *what() const throw() { return msg.c_str(); }

  CommError(const std::string &s) { msg = s; }
};

class ParameterError : public std::exception {
  std::string msg = "";

public:
  virtual const char *what() const throw() { return msg.c_str(); }

  ParameterError(const std::string &s) { msg = s; }
};

void throw_parameter_error(const std::string &s) {
  auto msg = std::string("Parameter error: ") + s;
  throw ParameterError(msg);
}

void throw_comm_error(const std::string &s) {
  auto msg = std::string("Communication error: ") + s;
  throw CommError(msg);
}

void sleep_ms(int t_ms) { std::this_thread::sleep_for(std::chrono::milliseconds(t_ms)); }

} // namespace qserver

#endif // INCLUDE_QSERVER_UTILS_
