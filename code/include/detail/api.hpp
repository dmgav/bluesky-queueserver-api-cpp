#ifndef INCLUDE_QSERVER_API_
#define INCLUDE_QSERVER_API_

#include <string>

#include <detail/basecomm.hpp>
#include <detail/defaults.hpp>
#include <detail/utils.hpp>

namespace qserver {

class REManagerAPI {
  BaseComm &comm;

  std::string http_provider;
  std::string refresh_token;
  std::string empty_string;

  int status_expiration_period = DEFAULT_STATUS_EXPIRATION_PERIOD; // in ms
  RequestResult status_current;
  std::list<std::function<void(const RequestResult &)>> status_get_cb;
  std::list<std::function<bool(const RequestResult &)>> wait_cb;
  std::mutex mtx_status_get_cb_lock;

  using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
  using Clock = std::chrono::steady_clock;
  using DurationMs = std::chrono::duration<float, std::milli>;
  TimePoint status_timestamp{};

  // std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> status_timestamp;
  //  std::chrono::time_point<std::chrono::system_clock> status_timestamp;
  std::thread thread_status_get;
  std::atomic<bool> is_closing{false};
  std::mutex mtx_event_status_get;
  std::condition_variable cv_event_status_get;

  // Cached queue
  std::string current_plan_queue_uid;
  json current_plan_queue;

  bool set_tokens_from_response(const RequestResult &response) {
    // Returns false if the response does not contain 'access_token' and/or 'refresh_token'
    //   and the error message needs to be changed.
    auto r_json = response.payload;
    bool return_value = true;

    if (r_json.contains("access_token") && r_json.contains("refresh_token")) {
      auto token = response.payload["access_token"].get<std::string>();
      auto refresh_token = response.payload["refresh_token"].get<std::string>();
      set_security_key(token, SecurityType::TOKEN);
      set_refresh_token(refresh_token);
      return true;
    } else
      return false;
  }

  RequestResult &process_request_result(RequestResult &result) {
    // Processing the response returned by 'send_request'. If 'success' is true,
    //   then check if 'success' in payload and set the success flag in the request
    //   to false and error message 'msg' to the message from the payload.
    if (result.success) {
      json &p = result.payload;
      if (p.is_object() && p.contains("status") && !p["status"].get<bool>()) {
        result.success = false;
        result.msg = p.contains("msg") ? p["msg"].get<std::string>() : "Unidentified error";
      }
    }
    return result;
  }

  template <typename T> RequestResult send_request_with_refresh(T f) {
    RequestResult result = f();
    bool refresh = false;
    if (!result.success && std::regex_search(result.msg, std::regex("Access token has expired")) &&
        (get_security_type() == SecurityType::TOKEN) && get_refresh_token().size())
      refresh = true;
    if (refresh) {
      session_refresh(get_refresh_token());
      result = f();
    }
    return result;
  }

  void thread_status_get_func() {
    std::unique_lock<std::mutex> lck(mtx_event_status_get);

    while (true) {
      if (cv_event_status_get.wait_for(lck, std::chrono::milliseconds(100)) != std::cv_status::timeout) {
        std::cout << "Attemting to load status ..." << std::endl;

        // Load status only if the status expiration period passed.
        if (Clock::now() - status_timestamp > DurationMs(status_expiration_period)) {
          std::cout << "Loading status ..." << std::endl;
          RequestResult result = load_status();
          // Update the timestamp only if communication was successful
          if (result.success and result.payload.size()) status_timestamp = Clock::now();
          status_current = std::move(result);
        };

        // Protect with lock until all callbacks are processed (until the end of the block)
        const std::lock_guard<std::mutex> lock(mtx_status_get_cb_lock);

        // Process callbacks related to 'status_get' requests. Each callbacks should be called only once.
        for (auto &f : status_get_cb) f(status_current);
        status_get_cb.clear();

        // Process callbacks related to 'wait' requests. Callbacks returning 'true' are removed from the list.
        auto it = wait_cb.begin();
        while (it != wait_cb.end()) {
          if ((*it)(status_current)) it = wait_cb.erase(it);
          else
            ++it;
        }
      }
      if (is_closing.load()) break;
    }
  }

  RequestResult load_status() { return send_request("status"); }

  void clear_status_timestamp() { status_timestamp = TimePoint(); }

  RequestResult status_get(bool reload = false) {
    std::mutex mtx;
    std::unique_lock<std::mutex> lk(mtx);
    std::condition_variable cv;
    RequestResult status;

    auto f = [&status, &cv](const RequestResult &rr) -> void {
      status = rr; // Copy
      cv.notify_all();
    };

    if (reload) clear_status_timestamp();

    {
      const std::lock_guard<std::mutex> lock(mtx_status_get_cb_lock);
      status_get_cb.push_back(std::function<void(const RequestResult &)>(f));
      cv_event_status_get.notify_all();
    }

    cv.wait(lk);

    return status;
  }

public:
  REManagerAPI(BaseComm &comm_) : comm(comm_) {
    thread_status_get = std::thread(&REManagerAPI::thread_status_get_func, this);
  }

  ~REManagerAPI() {
    is_closing = true;
    thread_status_get.join();
  }

  BaseComm &get_comm() { return comm; }
  bool is_zmq() { return comm.get_protocol() == Protocol::PROTOCOL_ZMQ; }
  bool is_http() { return comm.get_protocol() == Protocol::PROTOCOL_HTTP; }

  // HTTP provider

  const std::string &get_http_provider() { return http_provider; }

  void set_http_provider(const std::string &provider) {
    if (!is_http()) throw_parameter_error("Not allowed to set the provider: client is not in HTTP mode");
    http_provider = provider;
  }

  int get_status_expiration_period() { return status_expiration_period; }
  void set_status_expiration_period(int t_ms) {
    if (t_ms <= 0) throw_parameter_error("Status expiration period must be positive integer");
    status_expiration_period = t_ms;
  }

  // Security key and type (0MQ public key, HTTP token or API key).

  SecurityType get_security_type() { return comm.get_security_type(); }

  const std::string &get_security_key() { return comm.get_security_key(); }

  void set_security_key(const std::string &key, SecurityType security_type) {
    comm.set_security_key(key, security_type);
  }

  // Refresh token (only HTTP).

  const std::string &get_refresh_token() { return refresh_token; }

  void set_refresh_token(const std::string &refresh_token_) {
    if (!is_http()) throw_parameter_error("Not allowed to set the refresh token: client is not in HTTP mode");
    refresh_token = refresh_token_;
  }

  // ==================================================================================
  //                                send_single_request

  RequestResult send_single_request(const std::string &method, int timeout = 0) {
    RequestResult result = comm.send_request(method, timeout);
    return process_request_result(result);
  }

  RequestResult send_single_request(const std::string &method, const RequestParams &request_params,
                                    int timeout = 0) {
    RequestResult result = comm.send_request(method, request_params, timeout);
    return process_request_result(result);
  }

  RequestResult send_single_request(const Method &method, int timeout = 0) {
    RequestResult result = comm.send_request(method, timeout);
    return process_request_result(result);
  }

  RequestResult send_single_request(const Method &method, const RequestParams &request_params, int timeout = 0) {
    RequestResult result = comm.send_request(method, request_params, timeout);
    return process_request_result(result);
  };

  // ==================================================================================
  //                                    send_request

  RequestResult send_request(const std::string &method, int timeout = 0) {
    auto f = [this, &method, timeout]() { return send_single_request(method, timeout); };
    return send_request_with_refresh(f);
  }

  RequestResult send_request(const std::string &method, const RequestParams &request_params, int timeout = 0) {
    auto f = [this, &method, request_params, timeout]() {
      return send_single_request(method, request_params, timeout);
    };
    return send_request_with_refresh(f);
  }

  RequestResult send_request(const Method &method, int timeout = 0) {
    auto f = [this, &method, timeout]() { return send_single_request(method, timeout); };
    return send_request_with_refresh(f);
  }

  RequestResult send_request(const Method &method, const RequestParams &request_params, int timeout = 0) {
    auto f = [this, &method, request_params, timeout]() {
      return send_single_request(method, request_params, timeout);
    };
    return send_request_with_refresh(f);
  };

  // ===================================================================================================
  //                         API for authentication and authorization

  RequestResult login(const std::string &username, const std::string &password) {
    if (!is_http()) throw_parameter_error("API call is not allowed: 'login' API is supported only in HTTP mode.");

    RequestResult response;
    response.success = true;

    if (!http_provider.size()) {
      response.success = false;
      response.msg = "Provider is not set";
      return response;
    }

    if (response.success) {
      httplib::MultipartFormDataItems form_data = {
          {"username", username, "", ""},
          {"password", password, "", ""},
      };
      RequestParams rp(std::move(form_data));
      std::string path{std::string("/api/auth/provider") + http_provider};
      response = send_single_request(Method(HTTPMethod::HTTP_POST, path), rp);
    }

    if (response.success && !set_tokens_from_response(response)) {
      response.success = false;
      response.msg = "Parameters 'access_token' and/or 'refresh_token' are missing in the server response";
    }
    return response;
  }

  RequestResult session_refresh(const std::string &refresh_token_ = "") {
    if (!is_http()) throw_parameter_error("API call is not allowed: 'login' API is supported only in HTTP mode.");
    std::string r_token{refresh_token_.size() ? refresh_token_ : refresh_token};
    RequestResult response;
    if (r_token.size()) {
      auto params = json();
      params["refresh_token"] = r_token;
      response = send_single_request("session_refresh", RequestParams(params));
      if (response.success && !set_tokens_from_response(response)) {
        response.success = false;
        response.msg = "Parameters 'access_token' and/or 'refresh_token' are missing in the server response";
      }
    } else {
      response.success = false;
      response.msg = "Refresh token is not set or passed as a parameter";
    }
    return response;
  }

  RequestResult logout() {
    if (!is_http()) throw_parameter_error("API call is not allowed: 'login' API is supported only in HTTP mode.");
    auto response = send_request("logout");
    set_security_key("", SecurityType::NO_KEY);
    set_refresh_token("");
    return response;
  }

  // ===================================================================================================
  //                                          STATUS API

  RequestResult status(bool reload = false) { return status_get(reload); }

  // ===================================================================================================
  //                                          QUEUE API

  RequestResult queue_get(bool reload = false) {
    RequestResult status = status_get(reload), response;
    if (status.success) {
      std::string plan_queue_uid;
      try {
        plan_queue_uid = status.payload["plan_queue_uid"].get<std::string>();
      } catch (...) {
        response.success = false;
        response.msg = "Failed to load 'plan_queue_uid' from status JSON payload";
      }

      if (plan_queue_uid.size()) {
        if (plan_queue_uid.compare(current_plan_queue_uid)) {
          response = send_request("queue_get");
          if (response.success) {
            current_plan_queue_uid = plan_queue_uid;
            current_plan_queue = response.payload;
          }
        } else {
          response.success = true;
          response.msg = "";
          response.payload = current_plan_queue;
        }
      }

    } else {
      response = std::move(status);
    }
    return response;
  }
};

} // namespace qserver

#endif // INCLUDE_QSERVER_API_
