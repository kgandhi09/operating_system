# jk-timedated: org.freedesktop.timedate1 (time zone, setting the time,
# network time with BusyBox's ntpd) for KDE's Date & Time settings, which
# expect systemd's timedated.
cmake_minimum_required(VERSION 3.21)
project(jk-timedated VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core DBus)
add_executable(jk-timedated main.cpp timedate.cpp timedate.h)
target_compile_options(jk-timedated PRIVATE -Wall -Wextra)
target_link_libraries(jk-timedated PRIVATE Qt6::Core Qt6::DBus)
install(TARGETS jk-timedated RUNTIME DESTINATION sbin)
install(FILES org.freedesktop.timedate1.conf DESTINATION share/dbus-1/system.d)
