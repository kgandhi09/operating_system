# jk-dev: the dev session on tty1 (cli/jk-dev), one full-screen terminal,
# and what runs in it (session/jk-dev-session).
cmake_minimum_required(VERSION 3.21)
project(jk-dev LANGUAGES NONE)

install(PROGRAMS cli/jk-dev DESTINATION bin)
install(PROGRAMS session/jk-dev-session DESTINATION lib/jk_os)
install(FILES data/profile.d/jk-dev.sh DESTINATION /etc/profile.d)
