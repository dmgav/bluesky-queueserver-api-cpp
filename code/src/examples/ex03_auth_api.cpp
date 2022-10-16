#include <iostream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// /usr/lib/x86_64-linux-gnu/libzmq.so.5
// g++ test01.cpp -o test -lzmq

#include "qserver.hpp"

using qserver::HTTPComm;
using qserver::HTTPMethod;
using qserver::Method;
using qserver::REManagerAPI;
using qserver::RequestParams;
using qserver::SecurityType;
using qserver::sleep_ms;

int main() {
  HTTPComm comm{};
  REManagerAPI rm{comm};

  rm.set_http_provider("/toy/token");

  auto res = rm.login("bob", "bob_password");
  std::cout << "RESULT: " << res << "\n" << std::endl;

  res = rm.session_refresh();
  std::cout << "RESULT: " << res << "\n" << std::endl;

  sleep_ms(2000);

  res = rm.status();
  std::cout << "RESULT: " << res << "\n" << std::endl;

  sleep_ms(700);

  res = rm.queue_get();
  std::cout << "RESULT: " << res << "\n" << std::endl;

  res = rm.logout();
  std::cout << "RESULT: " << res << "\n" << std::endl;

  std::cout << "Completed!!!" << std::endl;
}
