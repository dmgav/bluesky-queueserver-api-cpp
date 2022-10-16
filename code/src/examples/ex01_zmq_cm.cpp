//#include "../cppzmq/zmq.hpp"
#include <exception>
#include <iostream>
#include <unistd.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// /usr/lib/x86_64-linux-gnu/libzmq.so.5
// g++ test01.cpp -o test -lzmq

#include "qserver.hpp"

using qserver::CommError;
using qserver::RequestParams;
using qserver::ZMQComm;

int main() {
  auto comm = ZMQComm();
  // comm.set_public_key("8U[K/inMw5e!z}v]Rh(Hpjt0JZGMAAp*]eYU</zg");

  // comm.connect();
  // comm.connect("abc");
  comm.connect("tcp://localhost:60695"); // Incorrect address
  // comm.disconnect();
  comm.connect("tcp://localhost:60615");
  comm.connect("tcp://localhost:60615");
  // comm.disconnect();
  // comm.disconnect();

  for (int i = 0; i < 10; ++i) {
    try {
      // std::string method = "queue_get";
      // auto params = json();
      auto response = comm.send_request("queue_get", RequestParams(json()));

      std::cout << "Response: " << response << std::endl;
    } catch (CommError e) { std::cout << "Exception: " << e.what() << std::endl; }
  }

  std::cout << "Completed!!!" << std::endl;
}
