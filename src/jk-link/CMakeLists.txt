# jk-link: this machine's identity on the network, finding the other jk_os
# machines and pairing with them (README.md). jk-linkd (daemon/, started by
# init/S45jk-link), jk-link (cli/) and its mesh console (gui/, jk-link gui),
# on the code they share (lib/).
cmake_minimum_required(VERSION 3.21)
project(jk-link VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(JK_LINK_GUI "Build the mesh console (needs Qt 6)" ON)

find_package(OpenSSL 3 REQUIRED)
find_package(Threads REQUIRED)

add_library(jklink STATIC lib/json.cpp lib/identity.cpp lib/beacon.cpp lib/net.cpp lib/store.cpp lib/ipc.cpp)
target_compile_options(jklink PRIVATE -Wall -Wextra)
target_link_libraries(jklink PUBLIC OpenSSL::SSL OpenSSL::Crypto)

add_executable(jk-linkd daemon/main.cpp daemon/discovery.cpp daemon/links.cpp daemon/control.cpp)
target_compile_options(jk-linkd PRIVATE -Wall -Wextra)
target_link_libraries(jk-linkd PRIVATE jklink Threads::Threads)

add_executable(jk-link-cli cli/main.cpp)
set_target_properties(jk-link-cli PROPERTIES OUTPUT_NAME jk-link)
target_compile_options(jk-link-cli PRIVATE -Wall -Wextra)
target_link_libraries(jk-link-cli PRIVATE jklink)

install(TARGETS jk-linkd RUNTIME DESTINATION sbin)
install(TARGETS jk-link-cli RUNTIME DESTINATION bin)
install(PROGRAMS init/S45jk-link DESTINATION /etc/init.d)

if(JK_LINK_GUI)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Qml Quick QuickControls2)
    qt_standard_project_setup(REQUIRES 6.5)
    qt_add_executable(jk-link-gui gui/main.cpp gui/linkclient.cpp gui/linkclient.h
        gui/meshview.cpp gui/meshview.h)
    qt_add_qml_module(jk-link-gui URI JkLink VERSION 1.0 QML_FILES gui/Main.qml)
    target_compile_options(jk-link-gui PRIVATE -Wall -Wextra)
    # (the generated QML type registration includes the headers by name)
    target_include_directories(jk-link-gui PRIVATE gui)
    target_link_libraries(jk-link-gui PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Qml Qt6::Quick
        Qt6::QuickControls2)
    # Started by jk-link gui: an internal program.
    install(TARGETS jk-link-gui RUNTIME DESTINATION lib/jk_os)
    install(FILES data/jk-link.desktop DESTINATION share/applications)
endif()
