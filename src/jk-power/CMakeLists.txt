# jk-power: the power settings (cli/jk-power), its window (gui/, jk-power
# gui) and the boot script that applies them.
cmake_minimum_required(VERSION 3.21)
project(jk-power LANGUAGES NONE)

install(PROGRAMS cli/jk-power DESTINATION sbin)
install(PROGRAMS init/S41power DESTINATION /etc/init.d)
install(FILES data/jk-power-gui.desktop DESTINATION share/applications)
add_subdirectory(gui)
