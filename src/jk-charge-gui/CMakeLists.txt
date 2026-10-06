# jk-charge-gui: battery monitoring and the controls offered by jk-charge.
cmake_minimum_required(VERSION 3.21)
project(jk-charge-gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)
qt_add_executable(jk-charge-gui app/main.cpp app/batterymonitor.cpp app/batterymonitor.h
    ../common/clisettings.cpp ../common/clisettings.h)
qt_add_qml_module(jk-charge-gui
    URI JkChargeGui
    VERSION 1.0
    QML_FILES app/Main.qml)
target_compile_options(jk-charge-gui PRIVATE -Wall -Wextra)
# (the generated QML type registration includes the header by name)
target_include_directories(jk-charge-gui PRIVATE app ../common)
target_link_libraries(jk-charge-gui PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
install(TARGETS jk-charge-gui RUNTIME DESTINATION bin)
install(FILES jk-charge-gui.desktop DESTINATION share/applications)
