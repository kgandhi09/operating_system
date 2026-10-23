# jk-gui: the KDE Plasma desktop on the console the user is logged in on
# (cli/jk-gui), and its session: jk-session (session/jk-session.c, setuid:
# opens the elogind session through PAM, data/pam.d/jk-gui) and
# jk-gui-session (session/jk-gui-session, what runs in it).
cmake_minimum_required(VERSION 3.21)
project(jk-gui LANGUAGES C)

find_library(PAM_LIBRARY pam REQUIRED)
add_executable(jk-session session/jk-session.c)
target_compile_options(jk-session PRIVATE -O2 -Wall -Wextra)
target_link_libraries(jk-session PRIVATE ${PAM_LIBRARY})
install(TARGETS jk-session RUNTIME DESTINATION lib/jk_os
        PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(PROGRAMS cli/jk-gui DESTINATION bin)
install(PROGRAMS session/jk-gui-session DESTINATION lib/jk_os)
install(FILES data/pam.d/jk-gui DESTINATION /etc/pam.d)
