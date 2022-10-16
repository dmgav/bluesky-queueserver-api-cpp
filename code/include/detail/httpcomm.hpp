#ifndef INCLUDE_QSERVER_HTTPCOMM_
#define INCLUDE_QSERVER_HTTPCOMM_

#include <string>

#include <detail/basecomm.hpp>
#include <detail/defaults.hpp>
#include <detail/utils.hpp>

namespace qserver {

const std::map<std::string, Method> method_mapping{
    {"ping", Method(HTTPMethod::HTTP_GET, "/api/ping")},
    {"status", Method(HTTPMethod::HTTP_GET, "/api/status")},
    {"queue_start", Method(HTTPMethod::HTTP_POST, "/api/queue/start")},
    {"queue_stop", Method(HTTPMethod::HTTP_POST, "/api/queue/stop")},
    {"queue_stop_cancel", Method(HTTPMethod::HTTP_POST, "/api/queue/stop/cancel")},
    {"queue_get", Method(HTTPMethod::HTTP_GET, "/api/queue/get")},
    {"queue_clear", Method(HTTPMethod::HTTP_POST, "/api/queue/clear")},
    {"queue_mode_set", Method(HTTPMethod::HTTP_POST, "/api/queue/mode/set")},
    {"queue_item_add", Method(HTTPMethod::HTTP_POST, "/api/queue/item/add")},
    {"queue_item_add_batch", Method(HTTPMethod::HTTP_POST, "/api/queue/item/add/batch")},
    {"queue_item_get", Method(HTTPMethod::HTTP_GET, "/api/queue/item/get")},
    {"queue_item_update", Method(HTTPMethod::HTTP_POST, "/api/queue/item/update")},
    {"queue_item_remove", Method(HTTPMethod::HTTP_POST, "/api/queue/item/remove")},
    {"queue_item_remove_batch", Method(HTTPMethod::HTTP_POST, "/api/queue/item/remove/batch")},
    {"queue_item_move", Method(HTTPMethod::HTTP_POST, "/api/queue/item/move")},
    {"queue_item_move_batch", Method(HTTPMethod::HTTP_POST, "/api/queue/item/move/batch")},
    {"queue_item_execute", Method(HTTPMethod::HTTP_POST, "/api/queue/item/execute")},
    {"history_get", Method(HTTPMethod::HTTP_GET, "/api/history/get")},
    {"history_clear", Method(HTTPMethod::HTTP_POST, "/api/history/clear")},
    {"environment_open", Method(HTTPMethod::HTTP_POST, "/api/environment/open")},
    {"environment_close", Method(HTTPMethod::HTTP_POST, "/api/environment/close")},
    {"environment_destroy", Method(HTTPMethod::HTTP_POST, "/api/environment/destroy")},
    {"re_pause", Method(HTTPMethod::HTTP_POST, "/api/re/pause")},
    {"re_resume", Method(HTTPMethod::HTTP_POST, "/api/re/resume")},
    {"re_stop", Method(HTTPMethod::HTTP_POST, "/api/re/stop")},
    {"re_abort", Method(HTTPMethod::HTTP_POST, "/api/re/abort")},
    {"re_halt", Method(HTTPMethod::HTTP_POST, "/api/re/halt")},
    {"re_runs", Method(HTTPMethod::HTTP_POST, "/api/re/runs")},
    {"plans_allowed", Method(HTTPMethod::HTTP_GET, "/api/plans/allowed")},
    {"devices_allowed", Method(HTTPMethod::HTTP_GET, "/api/devices/allowed")},
    {"plans_existing", Method(HTTPMethod::HTTP_GET, "/api/plans/existing")},
    {"devices_existing", Method(HTTPMethod::HTTP_GET, "/api/devices/existing")},
    {"permissions_reload", Method(HTTPMethod::HTTP_POST, "/api/permissions/reload")},
    {"permissions_get", Method(HTTPMethod::HTTP_GET, "/api/permissions/get")},
    {"permissions_set", Method(HTTPMethod::HTTP_POST, "/api/permissions/set")},
    {"script_upload", Method(HTTPMethod::HTTP_POST, "/api/script/upload")},
    {"function_execute", Method(HTTPMethod::HTTP_POST, "/api/function/execute")},
    {"task_status", Method(HTTPMethod::HTTP_GET, "/api/task/status")},
    {"task_result", Method(HTTPMethod::HTTP_GET, "/api/task/result")},
    {"lock", Method(HTTPMethod::HTTP_POST, "/api/lock")},
    {"unlock", Method(HTTPMethod::HTTP_POST, "/api/unlock")},
    {"lock_info", Method(HTTPMethod::HTTP_GET, "/api/lock/info")},
    {"manager_stop", Method(HTTPMethod::HTTP_POST, "/api/manager/stop")},
    {"manager_kill", Method(HTTPMethod::HTTP_POST, "/api/test/manager/kill")},
    // # API available only in HTTP version
    {"session_refresh", Method(HTTPMethod::HTTP_POST, "/api/auth/session/refresh")},
    {"apikey_new", Method(HTTPMethod::HTTP_POST, "/api/auth/apikey")},
    {"apikey_info", Method(HTTPMethod::HTTP_GET, "/api/auth/apikey")},
    {"apikey_delete", Method(HTTPMethod::HTTP_DELETE, "/api/auth/apikey")},
    {"whoami", Method(HTTPMethod::HTTP_GET, "/api/auth/whoami")},
    {"api_scopes", Method(HTTPMethod::HTTP_GET, "/api/auth/scopes")},
    {"logout", Method(HTTPMethod::HTTP_POST, "/api/auth/logout")},
};

class HTTPComm : public BaseComm {
  std::string host_address;
  std::string host_name;
  int host_port;

  SecurityType security_type = SecurityType::NO_KEY;
  std::string security_key;

  const std::string empty_string;

  std::pair<std::string, int> split_endpoint(const std::string &endpoint) {
    std::string host;
    int port;
    if (std::regex_search(endpoint, std::regex(":\\d+$"))) {
      auto n = endpoint.find_last_of(":");
      host = endpoint.substr(0, n);
      port = std::stoi(endpoint.substr(n + 1));
    } else {
      host = endpoint;
      port = DEFAULT_HTTP_HOST_PORT;
    }
    return std::make_pair(host, port);
  }

  Method expand_method(const std::string &method_) {
    auto it = method_mapping.find(method_);
    if (it == method_mapping.end()) throw_parameter_error("Method '" + method_ + "' is not supported by HTTP API");
    return (*it).second;
  }

  void initialize() {
    protocol = Protocol::PROTOCOL_HTTP;
    set_timeout(DEFAULT_HTTP_TIMEOUT);
  }

  void set_host_and_port(const std::string &address) {
    auto name_and_port = split_endpoint(address);
    host_address = address;
    host_name = name_and_port.first;
    host_port = name_and_port.second;
  }

public:
  HTTPComm() {
    initialize();
    set_host_and_port(DEFAULT_HTTP_HOST_ADDRESS);
  }

  HTTPComm(const std::string &address) {
    initialize();
    set_host_and_port(address);
  }

  const std::string &get_host_address() { return host_address; }
  void set_host_address(const std::string &address) { set_host_and_port(address); }

  // There is no '0MQ public key' in HTTP version of the API.
  const std::string &get_security_key() { return security_key; }
  SecurityType get_security_type() { return security_type; }
  void set_security_key(const std::string &key, SecurityType security_type_) {
    // Once public key is set, it can be changed, but the encryption can not be disabled.
    if ((security_type_ != SecurityType::TOKEN) && (security_type_ != SecurityType::API_KEY) &&
        (security_type_ != SecurityType::NO_KEY))
      throw_parameter_error("Unsupported security type. Supported types: 'NO_KEY', 'TOKEN', 'API_KEY'");
    if (!key.size() && (security_type_ != SecurityType::NO_KEY))
      throw_parameter_error("The public key must be an empty string for security type 'NO_KEY'");

    security_key = key;
    security_type = security_type_;
  }

  void connect() {}
  void connect(const std::string &address) { set_host_address(address); }
  void disconnect() {}

  RequestResult send_request(const std::string &method_, int timeout = 0) {
    Method method_expanded = expand_method(method_);
    return send_request(method_expanded, RequestParams(), timeout);
  }

  RequestResult send_request(const std::string &method_, const RequestParams &request_params, int timeout = 0) {
    Method method_expanded = expand_method(method_);
    return send_request(method_expanded, request_params, timeout);
  }

  RequestResult send_request(const Method &method, int timeout = 0) {
    return send_request(method, RequestParams(), timeout);
  }

  RequestResult send_request(const Method &method, const RequestParams &request_params, int timeout = 0) {
    const json &params = request_params.params;
    httplib::Headers headers(request_params.headers); // It is likely that 'headers' is modified.
    const httplib::MultipartFormDataItems &form_data = request_params.form_data;

    httplib::ClientImpl cli(host_name, host_port);

    int tout = timeout ? timeout : get_timeout();
    cli.set_connection_timeout(std::chrono::milliseconds(tout));
    cli.set_write_timeout(std::chrono::milliseconds(tout));
    cli.set_read_timeout(std::chrono::milliseconds(tout));

    bool send_form = form_data.size();

    std::string body;
    bool comm_error = false;

    std::string http_method;

    auto process_result = [](const httplib::Result &result) -> RequestResult {
      std::string body, msg;
      json payload;

      bool success = bool(result); // Indicates communication error

      if (success) {
        body = (*result).body;
        success = bool(body.size());
      } else {
        msg = std::string("Communication error (") + httplib::to_string(result.error()) + ")";
      }

      if (success) {
        try {
          payload = json::parse(body);
        } catch (...) {
          success = false;
          msg = "Failed to convert response to JSON: '" + body + "'";
        }
      }

      if (success && payload.is_object() && payload.contains("detail") && payload["detail"].is_string()) {
        success = false;
        msg = std::string("HTTP request failed: ") + payload["detail"].get<std::string>();
      }

      RequestResult res;
      res.payload = std::move(payload);
      res.success = success;
      res.msg = std::move(msg);

      return res;
    };

    // Add 'Authorization' header if a token or an API key is defined
    std::string auth_str;
    if (security_type == SecurityType::TOKEN) auth_str = std::string("Bearer " + security_key);
    else if (security_type == SecurityType::API_KEY)
      auth_str = std::string("ApiKey " + security_key);
    if (auth_str.size() and (headers.find("Authorization") == headers.end())) {
      headers.emplace(std::make_pair(std::string("Authorization"), auth_str));
    }

    RequestResult request_result;

    switch (method.http_method) {
      case HTTPMethod::HTTP_GET:
        {
          httplib::Request req;
          req.method = "GET";
          req.path = method.path;
          req.headers = headers;
          if (params.size()) {
            req.body = params.dump();
            req.headers.emplace("Content-Type", "application/json");
          }

          auto result = cli.send(req);
          request_result = process_result(result);
        }
        break;

      case HTTPMethod::HTTP_POST:
        {
          if (send_form) {
            auto result = cli.Post(method.path, form_data);
            request_result = process_result(result);
          } else {
            auto body_send = params.dump();
            auto result = cli.Post(method.path, headers, body_send, "application/json");
            request_result = process_result(result);
          }
        }
        break;
      case HTTPMethod::HTTP_DELETE:
        {
          auto body_send = params.dump();
          auto result = cli.Delete(method.path, headers, body_send, "application/json");
          request_result = process_result(result);
        }
        break;
      default:
        throw_comm_error("Unknown HTTP method");
    }

    return request_result;
  }
};

} // namespace qserver

#endif // INCLUDE_QSERVER_HTTPCOMM_
