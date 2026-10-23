# jk-cuda: the CUDA toolkit, installed on demand (cli/jk-cuda), and
# prime-run (cli/prime-run): a program on the NVIDIA GPU while the desktop
# stays on the built-in one.
cmake_minimum_required(VERSION 3.21)
project(jk-cuda LANGUAGES NONE)

install(PROGRAMS cli/jk-cuda DESTINATION sbin)
install(PROGRAMS cli/prime-run DESTINATION bin)
install(FILES data/profile.d/jk-cuda.sh DESTINATION /etc/profile.d)
