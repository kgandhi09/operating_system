#include "sensor.h"
#include <iostream>
#include <jk/runtime.hpp>
int main(int argc, char **argv) {
  try {
    if (argc < 3 || argc > 4)
      throw jk::Error("usage: jk_subscriber DOMAIN NODE_NAME [COUNT]");
    int count = argc == 4 ? std::stoi(argv[3]) : 10;
    if (count < 1 || count > 100000)
      throw jk::Error("COUNT must be 1..100000");
    jk::Node node(argv[1], argv[2]);
    auto sub = node.subscriber<jkbuf_demo_SensorSample>("/demo/sensor");
    std::cout << "READY " << argv[2] << '\n' << std::flush;
    for (int i = 0; i < count; ++i) {
      auto loan = sub.take(std::chrono::seconds(10));
      if (!loan)
        throw jk::Error("timed out waiting for publication");
      std::cout << "RECEIVED " << (*loan)->f_sequence
                << " value=" << (*loan)->f_value << " slot=" << loan->slot()
                << " generation=" << loan->generation()
                << " address=" << static_cast<const void *>(&**loan) << '\n'
                << std::flush;
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "subscriber: " << e.what() << '\n';
    return 1;
  }
}
