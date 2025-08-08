cmake_minimum_required(VERSION 3.5)

project(FileProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/fileprocessor.cpp
)

set(HEADERS
    include/mainwindow.h
    include/fileprocessor.h
)

set(FORMS
    ui/mainwindow.ui
)

set(RESOURCES
    resources/resources.qrc
)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/ui
)

add_executable(FileProcessor
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

target_link_libraries(FileProcessor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)

target_include_directories(FileProcessor PRIVATE
    include
    ${CMAKE_BINARY_DIR} 
)

install(TARGETS FileProcessor
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)
