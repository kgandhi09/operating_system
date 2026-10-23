# jk-install: installing jk_os and its first boot. jk-install (to a disk),
# jk-setup (first boot: accounts and settings), jk-live (the live system),
# jk-getty (the consoles' login), and the account helpers they share
# (lib/accounts.sh).
cmake_minimum_required(VERSION 3.21)
project(jk-install LANGUAGES NONE)

install(PROGRAMS cli/jk-install cli/jk-setup cli/jk-live cli/jk-getty DESTINATION /sbin)
install(FILES lib/accounts.sh DESTINATION lib/jk_os)
