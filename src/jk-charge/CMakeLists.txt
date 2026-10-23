# jk-charge: how the battery charges (cli/jk-charge), its window
# (gui/, jk-charge gui) and the boot script that applies the saved choice.
cmake_minimum_required(VERSION 3.21)
project(jk-charge LANGUAGES NONE)

install(PROGRAMS cli/jk-charge DESTINATION sbin)
install(PROGRAMS init/S08charge DESTINATION /etc/init.d)
install(FILES data/jk-charge-gui.desktop DESTINATION share/applications)
add_subdirectory(gui)
