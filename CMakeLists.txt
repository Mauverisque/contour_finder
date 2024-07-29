cmake_minimum_required(VERSION 3.5.0)
project(contours VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Sql)

add_library(contourfinder SHARED
    src/main_window.cpp
    src/contour_detection.cpp
    src/clickable_label.cpp
    src/sql_query_handler.cpp
    include/main_window.h
    include/contour_detection.h
    include/clickable_label.h
    include/sql_query_handler.h
)

target_include_directories(contourfinder PUBLIC include)

target_link_libraries(contourfinder
    ${OpenCV_LIBS}
    Qt::Core
    Qt::Widgets
    Qt::Sql
    pqxx
)

add_executable(contours src/main.cpp)
target_link_libraries(contours contourfinder)
