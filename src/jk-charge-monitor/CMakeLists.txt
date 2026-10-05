# jk-charge-monitor: the battery and its charger in real time (Qt Quick),
# from /sys/class/power_supply. Charging settings: jk-charge (tablet).
cmake_minimum_required(VERSION 3.21)
project(jk-charge-monitor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)
qt_add_executable(jk-charge-monitor app/main.cpp app/batterymonitor.cpp app/batterymonitor.h)
qt_add_qml_module(jk-charge-monitor
    URI JkChargeMonitor
    VERSION 1.0
    QML_FILES app/Main.qml)
target_compile_options(jk-charge-monitor PRIVATE -Wall -Wextra)
# (the generated QML type registration includes the header by name)
target_include_directories(jk-charge-monitor PRIVATE app)
target_link_libraries(jk-charge-monitor PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
install(TARGETS jk-charge-monitor RUNTIME DESTINATION bin)
install(FILES jk-charge-monitor.desktop DESTINATION share/applications)
