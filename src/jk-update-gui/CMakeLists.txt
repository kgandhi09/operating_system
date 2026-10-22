# jk-update-gui: Software Update (jk-update gui) and the update notification
# in the desktop session (jk-update gui --watch).
cmake_minimum_required(VERSION 3.21)
project(jk-update-gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2)
find_package(KF6WidgetsAddons REQUIRED)
find_package(KF6Notifications REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(jk-update-gui app/main.cpp app/updater.cpp app/updater.h app/notifier.cpp app/notifier.h
    ../common/adminpassword.cpp ../common/adminpassword.h)
qt_add_qml_module(jk-update-gui URI JkUpdateGui VERSION 1.0 QML_FILES app/Main.qml)
# (the generated QML type registration includes the header by name)
target_include_directories(jk-update-gui PRIVATE app ../common)
target_link_libraries(jk-update-gui PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Qml Qt6::Quick
    Qt6::QuickControls2 KF6::WidgetsAddons KF6::Notifications)
target_compile_options(jk-update-gui PRIVATE -Wall -Wextra)
# Started by jk-update gui, not on its own: an internal program.
install(TARGETS jk-update-gui RUNTIME DESTINATION lib/jk_os)
install(FILES jk-update-gui.desktop DESTINATION share/applications)
# The notification's event (KNotification's "updateAvailable").
install(FILES jk-update.notifyrc DESTINATION share/knotifications6)
