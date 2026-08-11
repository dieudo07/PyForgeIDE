cmake_minimum_required(VERSION 3.20)
project(PyForgeIDE VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_compile_options(/O2 /W3)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Network WebSockets Concurrent PrintSupport)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/core/SyntaxHighlighter.cpp
    src/core/AutoComplete.cpp
    src/core/LSPClient.cpp
    src/ui/MainWindow.cpp
    src/ui/EditorWidget.cpp
    src/ui/GutterWidget.cpp
    src/ui/FileExplorer.cpp
    src/ui/Terminal.cpp
    src/utils/Theme.cpp
)

set(RESOURCES
    resources/resources.qrc
)

# Icône Windows
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.rc")
endif()

qt_add_executable(PyForgeIDE
    WIN32
    ${SOURCES}
    ${RESOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_include_directories(PyForgeIDE PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(PyForgeIDE PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Concurrent
    Qt6::PrintSupport
    ${Python3_LIBRARIES}
)