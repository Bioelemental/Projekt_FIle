cmake_minimum_required(VERSION 3.16)

project(FileOrganizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Test)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    llmclient.h
    llmclient.cpp
    scriptrunner.h
    scriptrunner.cpp
)

qt_add_executable(FileOrganizer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(FileOrganizer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

qt_finalize_executable(FileOrganizer)

# Testy jednostkowe
enable_testing()

qt_add_executable(FileOrganizerTests
    tests/test_main.cpp
    scriptrunner.cpp
    scriptrunner.h
    llmclient.cpp
    llmclient.h
)

target_link_libraries(FileOrganizerTests PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME FileOrganizerTests COMMAND FileOrganizerTests)