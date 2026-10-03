# jk-viz: every process and what it consumes, as layers from the silicon
# (top) to the applications (bottom). Two programs:
#   jk-vizd  the collector (root, a service: /etc/init.d/S50jk-vizd)
#   jk-viz   the desktop app (Qt Quick), which shows what jk-vizd sends
cmake_minimum_required(VERSION 3.21)
project(jk-viz VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(JK_VIZ_APP "Build the desktop app (needs Qt 6)" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)
find_package(Threads REQUIRED)

add_executable(jk-vizd
    daemon/main.cpp daemon/collector.cpp daemon/json.cpp daemon/gpu.cpp
    daemon/netcap.cpp daemon/dbusnames.cpp daemon/util.cpp)
target_compile_options(jk-vizd PRIVATE -Wall -Wextra)
target_link_libraries(jk-vizd PRIVATE PkgConfig::DBUS Threads::Threads ${CMAKE_DL_LIBS})
install(TARGETS jk-vizd RUNTIME DESTINATION sbin)

if(JK_VIZ_APP)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Qml Quick QuickControls2)
    qt_standard_project_setup(REQUIRES 6.5)
    qt_add_executable(jk-viz
        app/main.cpp app/client.cpp app/client.h app/graphmodel.cpp app/graphmodel.h
        app/graphview.cpp app/graphview.h)
    qt_add_qml_module(jk-viz
        URI JkViz
        VERSION 1.0
        QML_FILES app/Main.qml app/SidePanel.qml)
    target_compile_options(jk-viz PRIVATE -Wall -Wextra)
    # (the generated QML type registration includes the headers by name)
    target_include_directories(jk-viz PRIVATE app)
    target_link_libraries(jk-viz PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Qml Qt6::Quick Qt6::QuickControls2)
    install(TARGETS jk-viz RUNTIME DESTINATION bin)
    install(FILES jk-viz.desktop DESTINATION share/applications)
endif()
