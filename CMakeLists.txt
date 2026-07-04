cmake_minimum_required(VERSION 3.24)
project(notwork LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)
pkg_check_modules(SNDFILE   REQUIRED IMPORTED_TARGET sndfile)

set(NOTWORK_SOURCES
    src/main.cpp
    src/app/MainWindow.cpp
    src/app/MainWindow.h
    src/app/Settings.cpp
    src/app/Settings.h
    src/app/SettingsDialog.cpp
    src/app/SettingsDialog.h
    src/engine/AudioEngine.cpp
    src/engine/AudioEngine.h
    src/engine/DeviceList.cpp
    src/engine/DeviceList.h
    src/engine/PortAudioSession.cpp
    src/engine/PortAudioSession.h
    src/gui/Style.cpp
    src/gui/Style.h
    src/gui/TransportBar.cpp
    src/gui/TransportBar.h
)

qt_add_executable(notwork MACOSX_BUNDLE ${NOTWORK_SOURCES})

target_include_directories(notwork PRIVATE src)

target_link_libraries(notwork PRIVATE
    Qt6::Widgets
    PkgConfig::PORTAUDIO
    PkgConfig::SNDFILE
)

target_compile_options(notwork PRIVATE -Wall -Wextra -Wpedantic)

set_target_properties(notwork PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Notwork"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.notwork.Notwork"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
)
