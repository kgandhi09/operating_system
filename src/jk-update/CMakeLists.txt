# jk-update: OS updates (cli/jk-update), the release watcher
# (cli/jk-update-watch, started by init/S60jk-update), and the update window
# and notification in the desktop session (gui/).
cmake_minimum_required(VERSION 3.21)
project(jk-update LANGUAGES NONE)

install(PROGRAMS cli/jk-update cli/jk-update-watch DESTINATION sbin)
install(PROGRAMS init/S60jk-update DESTINATION /etc/init.d)
install(FILES data/jk-update-gui.desktop DESTINATION share/applications)
# The notification's event (KNotification's "updateAvailable").
install(FILES data/jk-update.notifyrc DESTINATION share/knotifications6)
install(FILES data/jk-update-notify.desktop DESTINATION /etc/xdg/autostart)
add_subdirectory(gui)
install(FILES data/profile.d/jk-update.sh DESTINATION /etc/profile.d)
