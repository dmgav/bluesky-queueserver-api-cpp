#ifndef INCLUDE_QSERVER_BASECOMM_
#define INCLUDE_QSERVER_BASECOMM_

#include <string>

#include <detail/defaults.hpp>
#include <detail/utils.hpp>

namespace qserver {

class BaseComm {
  int default_timeout = DEFAULT_ZMQ_TIMEOUT;

protected:
  Protocol protocol = Protocol::PROTOCOL_NONE;

public:
  Protocol get_protocol() { return protocol; }

  int get_timeout() { return default_timeout; }
  void set_timeout(int t) { default_timeout = t; }

  virtual const std::string &get_host_address() = 0;
  virtual void set_host_address(const std::string &address) = 0;

  virtual const std::string &get_security_key() = 0;
  virtual SecurityType get_security_type() = 0;
  virtual void set_security_key(const std::string &key, SecurityType security_type_) = 0;

  virtual void connect() = 0;
  virtual void connect(const std::string &address) = 0;
  virtual void disconnect() = 0;

  virtual RequestResult send_request(const std::string &method_, int timeout = 0) = 0;
  virtual RequestResult send_request(const std::string &method_, const RequestParams &request_params,
                                     int timeout = 0) = 0;
  virtual RequestResult send_request(const Method &method, int timeout = 0) = 0;
  virtual RequestResult send_request(const Method &method, const RequestParams &request_params,
                                     int timeout = 0) = 0;
};

} // namespace qserver

#endif // INCLUDE_QSERVER_BASECOMM_
