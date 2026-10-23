# jk-apt: Debian packages in a container of their own on the data partition.
# cli/apt stands for apt, apt-get, apt-cache, apt-mark, apt-run, dpkg and
# dpkg-query (links in /usr/bin, by the name it is called with);
# add-apt-repository; apt-setup (root: makes the container).
cmake_minimum_required(VERSION 3.21)
project(jk-apt LANGUAGES NONE)

install(PROGRAMS cli/apt cli/add-apt-repository DESTINATION lib/jk_os)
install(PROGRAMS cli/apt-setup DESTINATION sbin)
# jk_install_link(<target in /usr/lib/jk_os> <names...>): /usr/bin/<name> links.
function(jk_install_link target)
    foreach(name ${ARGN})
        install(CODE "file(MAKE_DIRECTORY \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin\")
            file(CREATE_LINK ../lib/jk_os/${target} \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${name}\" SYMBOLIC)")
    endforeach()
endfunction()
jk_install_link(apt apt apt-get apt-cache apt-mark apt-run dpkg dpkg-query)
jk_install_link(add-apt-repository add-apt-repository apt-add-repository)
install(FILES data/profile.d/jk-apt.sh DESTINATION /etc/profile.d)
