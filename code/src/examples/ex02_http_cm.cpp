//#include "../cppzmq/zmq.hpp"
#include <exception>
#include <iostream>
#include <unistd.h>

//#include "../cpp-httplib/httplib.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// /usr/lib/x86_64-linux-gnu/libzmq.so.5
// g++ test01.cpp -o test -lzmq

#include "qserver.hpp"

using qserver::HTTPComm;
using qserver::HTTPMethod;
using qserver::Method;
using qserver::RequestParams;
using qserver::SecurityType;

int main() {
  auto comm = HTTPComm();

  auto res = comm.send_request("status");
  std::cout << "RESULT: " << res << "\n" << std::endl;

  httplib::MultipartFormDataItems form_data = {
      {"username", "bob", "", ""},
      {"password", "bob_password", "", ""},
  };

  std::string provider{"/toy/token"};
  res = comm.send_request(Method(HTTPMethod::HTTP_POST, std::string("/api/auth/provider") + provider),
                          RequestParams(form_data));

  std::cout << "RESULT: " << res << "\n" << std::endl;

  std::string token = res.payload["access_token"].get<std::string>();
  std::cout << "token: " << token << std::endl;

  //httplib::Headers headers = {{"Authorization", std::string("Bearer " + token)}};

  comm.set_security_key(token, SecurityType::TOKEN);
  res = comm.send_request("queue_get", RequestParams(json()));

  std::cout << "RESULT: " << res << "\n" << std::endl;

  auto params_plan = json();
  params_plan["item"]["item_type"] = "plan";
  params_plan["item"]["name"] = "count";
  params_plan["item"]["args"] = json::parse("[[\"det1\", \"det2\"]]");
  params_plan["item"]["kwargs"] = json::parse("{\"num\": 10, \"delay\": 1}");

  std::cout << "Plan is ready: " << params_plan << std::endl;

  res = comm.send_request("queue_item_add", RequestParams(params_plan));

  std::cout << "RESULT: " << res << "\n" << std::endl;

  std::cout << "Requesting status ..." << std::endl;
  auto req_params = json::parse("{\"task_uid\": \"abc\"}");
  res = comm.send_request("task_status", RequestParams(req_params));
  std::cout << "RESULT: " << res << "\n"
            << "\n"
            << std::endl;

  std::cout << "Sleeping ..." << std::endl;
  req_params = json::parse("{\"time\": 1}");
  res = comm.send_request(Method(HTTPMethod::HTTP_GET, "/api/test/server/sleep"),
                          RequestParams(req_params), 1500);
  std::cout << "RESULT: " << res << "\n"
            << "\n"
            << std::endl;

  std::cout << "Completed!!!" << std::endl;
}
