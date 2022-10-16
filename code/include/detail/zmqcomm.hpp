#ifndef INCLUDE_QSERVER_ZMQCOMM_
#define INCLUDE_QSERVER_ZMQCOMM_

#include <string>

#include <detail/basecomm.hpp>
#include <detail/defaults.hpp>
#include <detail/utils.hpp>

namespace qserver {

class ZMQComm : public BaseComm {
  std::string host_address = DEFAULT_ZMQ_SERVER_ADDRESS;
  std::string zmq_public_key = "";

  zmq::context_t ctx;
  zmq::socket_t sock;

  bool zmq_error = false;
  bool zmq_connected = false;

  void socket_open() {
    sock = zmq::socket_t(ctx, zmq::socket_type::req);
    sock.set(zmq::sockopt::sndtimeo, DEFAULT_ZMQ_SNDTIMEO);
    sock.set(zmq::sockopt::rcvtimeo, DEFAULT_ZMQ_RCVTIMEO);
    sock.set(zmq::sockopt::linger, DEFAULT_ZMQ_LINGER);
    socket_enable_encryption();
    socket_connect();
    zmq_error = false;
  }

  void socket_enable_encryption() {
    if (zmq_public_key.size()) {
      sock.set(zmq::sockopt::curve_serverkey, zmq_public_key);
      sock.set(zmq::sockopt::curve_publickey, FIXED_ZMQ_PUBLIC_KEY);
      sock.set(zmq::sockopt::curve_secretkey, FIXED_ZMQ_PRIVATE_KEY);
    }
  }

  void socket_connect(bool always_connect = false) {
    if (always_connect || zmq_connected) {
      zmq_connected = false;
      sock.connect(host_address); // Exception is thrown if fails
      zmq_connected = true;
    }
  }

  void socket_reset() {
    // Reset the socket after an error.
    sock.close();
    socket_open();
  }

  void register_comm_error(const std::string &s) {
    zmq_error = true;
    throw_comm_error(s);
  }

  void initialize() {
    protocol = Protocol::PROTOCOL_ZMQ;
    set_timeout(DEFAULT_ZMQ_TIMEOUT);
    sock = zmq::socket_t(ctx, zmq::socket_type::req);
    socket_open();
  }

public:
  ZMQComm() { initialize(); }

  ZMQComm(const std::string &address) {
    set_host_address(address);
    initialize();
  }

  const std::string &get_host_address() { return host_address; }
  void set_host_address(const std::string &address) { host_address = address; }

  const std::string &get_security_key() { return zmq_public_key; }
  SecurityType get_security_type() {
    SecurityType t = SecurityType::NO_KEY;
    if (zmq_public_key.size()) t = SecurityType::PUBLIC_KEY;
    return t;
  };
  void set_security_key(const std::string &key, SecurityType security_type_) {
    // Once public key is set, it can be changed, but the encryption can not be disabled.
    if (security_type_ != SecurityType::PUBLIC_KEY)
      throw_parameter_error("Unsupported security type. Supported type: 'PUBLIC_KEY'");
    if (!key.size()) throw_parameter_error("The public key is an empty string. Security cannot be disabled");
    zmq_public_key = key;
    socket_enable_encryption();
  }

  void connect() {
    if (zmq_connected) disconnect();
    try {
      socket_connect(true);
    } catch (...) {
      auto msg = std::string("Failed to connect to the server '") + host_address + "'";
      register_comm_error(msg);
    }
  }

  void connect(const std::string &address) {
    if (zmq_connected) disconnect(); // Diconnect before the address is changed
    host_address = address;
    connect();
  }

  void disconnect() {
    if (zmq_connected) {
      zmq_connected = false;
      sock.disconnect(host_address);
    }
  }

  RequestResult send_request(const Method &method, int timeout = 0) {
    throw_parameter_error("send_request(const Method &method, int timeout = 0) is not supported for 0MQ requests");
    return RequestResult();
  }

  RequestResult send_request(const Method &method, const RequestParams &request_params, int timeout = 0) {
    throw_parameter_error("send_request(const Method &method, const RequestParams &request_params, int timeout = "
                          "0) is not supported for 0MQ requests");
    return RequestResult();
  }

  RequestResult send_request(const std::string &method_, int timeout = 0) {
    return send_request(method_, RequestParams(), timeout);
  }

  RequestResult send_request(const std::string &method, const RequestParams &request_params, int timeout = 0) {
    if (!zmq_connected) throw_comm_error("Socket is not connected");
    const json &params = request_params.params;

    auto payload = json();
    payload["method"] = method;
    if (!params.empty()) payload["params"] = params;

    if (zmq_error) socket_reset();

    auto s = payload.dump();
    auto buffer = zmq::buffer(s.c_str(), s.size());
    sock.send(buffer, zmq::send_flags::dontwait);

    zmq::pollitem_t p = {(void *)sock, 0, ZMQ_POLLIN, 0};
    if (!zmq::poll(&p, 1, std::chrono::milliseconds(timeout ? timeout : get_timeout())))
      register_comm_error("Timeout occurred while waiting for response from the server");

    zmq::message_t zmq_msg;
    auto recv_res = sock.recv(zmq_msg);
    if (not recv_res.has_value()) register_comm_error("Failed to receive data");

    auto process_result = [](const zmq::message_t &zmq_msg) -> RequestResult {
      RequestResult res;
      auto body = zmq_msg.to_string();
      try {
        res.payload = json::parse(body);
        res.success = true;
        res.msg = "";
      } catch (...) {
        res.payload = json();
        res.success = false;
        res.msg = "Failed to convert response to JSON: '" + body + "'";
      }
      return res;
    };

    return process_result(zmq_msg);
  }
};

} // namespace qserver

#endif // INCLUDE_QSERVER_ZMQCOMM_
