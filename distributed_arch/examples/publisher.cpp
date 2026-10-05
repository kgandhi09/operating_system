#include "sensor.h"
#include <cstring>
#include <iostream>
#include <jk/runtime.hpp>
#include <thread>
int main(int argc, char **argv) {
  try {
    if (argc < 2 || argc > 3)
      throw jk::Error("usage: jk_publisher DOMAIN [COUNT]");
    int count = argc == 3 ? std::stoi(argv[2]) : 10;
    if (count < 1 || count > 100000)
      throw jk::Error("COUNT must be 1..100000");
    jk::Node node(argv[1], "sensor-publisher");
    auto pub = node.publisher<jkbuf_demo_SensorSample>("/demo/sensor");
    for (int i = 0; i < count; ++i) {
      auto loan = pub.loan();
      if (!loan)
        throw jk::Error("shared pool exhausted: subscriber or writer is "
                        "retaining messages");
      (*loan)->f_sequence = i;
      (*loan)->f_value = i * 0.5;
      (*loan)->f_frame.length = 5;
      std::memcpy((*loan)->f_frame.data, "front", 5);
      (*loan)->f_readings.length = 2;
      (*loan)->f_readings.data[0] = 1.25f;
      (*loan)->f_readings.data[1] = 2.5f;
      auto slot = loan->slot();
      auto generation = loan->generation();
      pub.publish(std::move(*loan));
      std::cout << "PUBLISHED " << i << " slot=" << slot
                << " generation=" << generation << '\n'
                << std::flush;
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "publisher: " << e.what() << '\n';
    return 1;
  }
}
