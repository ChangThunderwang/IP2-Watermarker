cmake_minimum_required(VERSION 3.19)
project(IP2Watermarker)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_PREFIX_PATH "/Users/shein/Qt/5.15.2/clang_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
include_directories(.)

add_executable(IP2Watermarker
        dragframe.cpp
        dragframe.h
        draglabel.cpp
        draglabel.h
        logooverlay.cpp
        logooverlay.h
        main.cpp
        mainwindow.cpp
        mainwindow.h
        placeholdersquare.cpp
        placeholdersquare.h
        mainwindow.ui
        DragEventFilter.cpp DragEventFilter.h)

target_link_libraries(IP2Watermarker Qt5::Widgets)