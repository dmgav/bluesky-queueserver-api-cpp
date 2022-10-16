#ifndef INCLUDE_QSERVER_DEFAULTS_
#define INCLUDE_QSERVER_DEFAULTS_

namespace qserver {

#include <string>

const std::string DEFAULT_ZMQ_SERVER_ADDRESS = "tcp://localhost:60615";
constexpr int DEFAULT_ZMQ_RCVTIMEO = 500;
constexpr int DEFAULT_ZMQ_SNDTIMEO = 500;
constexpr int DEFAULT_ZMQ_LINGER = 100;
constexpr int DEFAULT_ZMQ_TIMEOUT = 2000; // Communication timeout

const std::string FIXED_ZMQ_PUBLIC_KEY = "wt8[6a8eoXFRVL<l2JBbOzs(hcI%kRBIr0Do/eLC";
const std::string FIXED_ZMQ_PRIVATE_KEY = "=@e7WwVuz{*eGcnv{AL@x2hmX!z^)wP3vKsQ{S7s";

const std::string DEFAULT_HTTP_HOST_ADDRESS = "localhost:60610";
const int DEFAULT_HTTP_HOST_PORT = 60610;
constexpr int DEFAULT_HTTP_TIMEOUT = 2000; // Communication timeout

const int DEFAULT_STATUS_EXPIRATION_PERIOD = 500;

} // namespace qserver

#endif // INCLUDE_QSERVER_DEFAULTS_
