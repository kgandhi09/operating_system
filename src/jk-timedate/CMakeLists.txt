# jk-timedate: the clock for KDE's Date & Time settings. Its daemon
# jk-timedated (daemon/: org.freedesktop.timedate1, time zone, setting the
# time, network time), its D-Bus policy and the boot script that starts it.
cmake_minimum_required(VERSION 3.21)
project(jk-timedate LANGUAGES NONE)

install(PROGRAMS init/S19timedate DESTINATION /etc/init.d)
install(FILES data/org.freedesktop.timedate1.conf DESTINATION share/dbus-1/system.d)
add_subdirectory(daemon)
