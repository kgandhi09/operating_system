#include <csignal>
#include <iostream>
#include <jk/runtime.hpp>
#include <thread>
namespace {
volatile std::sig_atomic_t stopped = 0;
void stop(int) { stopped = 1; }
} // namespace
int main(int argc, char **argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--help") {
      std::cout << "usage: nodemaster DOMAIN | nodemaster --cleanup DOMAIN\n";
      return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--cleanup") {
      jk::LocalMaster::cleanup(argv[2]);
      return 0;
    }
    if (argc != 2)
      throw jk::Error("usage: nodemaster DOMAIN | nodemaster --cleanup DOMAIN");
    std::signal(SIGTERM, stop);
    std::signal(SIGINT, stop);
    jk::LocalMaster master(argv[1]);
    std::cout << "READY " << argv[1] << '\n' << std::flush;
    while (!stopped) {
      master.reap();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "nodemaster: " << e.what() << '\n';
    return 1;
  }
}
