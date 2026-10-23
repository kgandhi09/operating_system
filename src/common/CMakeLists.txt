# common: what the jk tools share. gui/ is compiled into their windows (the
# CLI settings bridge, the administrator password dialog); lib/ goes to
# /usr/lib/jk_os: jk-settings-gui (starts a settings window, also under
# sudo), jk-audio and gpu.sh (the desktop and dev sessions).
cmake_minimum_required(VERSION 3.21)
project(jk-common LANGUAGES NONE)

install(PROGRAMS lib/jk-settings-gui lib/jk-audio DESTINATION lib/jk_os)
install(FILES lib/gpu.sh DESTINATION lib/jk_os)
