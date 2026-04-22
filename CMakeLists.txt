cmake_minimum_required(VERSION 3.20)
project(WaterCurtain)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

add_executable(WaterCurtain
    main.cpp
    WaterCurtainWidget.cpp
    WaterCurtainWidget.h
)

target_link_libraries(WaterCurtain Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)