cmake_minimum_required(VERSION 3.21)

project(wring VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(ORB_SOURCES
    src/main.cpp
    src/core/wring_controller.cpp
    src/core/wring_controller.hpp
)

set(ORB_QML_FILES
    qml/Main.qml
    qml/Wring.qml
    qml/Ring.qml
    qml/RingItem.qml
    qml/WindowItem.qml
    qml/Ring2Button.qml
    qml/WorkspaceItem.qml
    qml/PopularAppItem.qml
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

if(WIN32)
    list(APPEND ORB_SOURCES
        src/platform/desktop_backend.hpp
        src/platform/windows/windows_backend.cpp
        src/platform/windows/windows_backend.hpp
        src/platform/windows/windows_window_manager.cpp
        src/platform/windows/windows_window_manager.hpp
        src/platform/windows/windows_input.cpp
        src/platform/windows/windows_input.hpp
        src/input/global_input.cpp
        src/input/global_input.hpp
    )
elseif(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11 xext)
    pkg_check_modules(XCB REQUIRED xcb xcb-util)

    list(APPEND ORB_SOURCES
        src/platform/desktop_backend.hpp
        src/platform/linux/x11/x11_backend.cpp
        src/platform/linux/x11/x11_backend.hpp
        src/platform/linux/x11/x11_window_manager.cpp
        src/platform/linux/x11/x11_window_manager.hpp
        src/platform/linux/x11/x11_input.cpp
        src/platform/linux/x11/x11_input.hpp
        src/input/global_input.cpp
        src/input/global_input.hpp
    )
endif()

set(QML_QRC_CONTENT "<RCC>\n  <qresource prefix=\"/\">\n")
foreach(qml_file ${ORB_QML_FILES})
    string(REPLACE "qml/" "" alias ${qml_file})
    set(QML_QRC_CONTENT "${QML_QRC_CONTENT}    <file alias=\"${alias}\">${qml_file}</file>\n")
endforeach()
set(QML_QRC_CONTENT "${QML_QRC_CONTENT}  </qresource>\n</RCC>\n")
file(WRITE ${CMAKE_BINARY_DIR}/qml.qrc "${QML_QRC_CONTENT}")

add_executable(wring ${ORB_SOURCES} ${CMAKE_BINARY_DIR}/qml.qrc)

target_include_directories(wring PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/platform
    ${CMAKE_SOURCE_DIR}/src/input
)

target_link_libraries(wring PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
)

if(WIN32)
    target_link_libraries(wring PRIVATE user32 shlwapi)
elseif(UNIX AND NOT APPLE)
    target_include_directories(wring PRIVATE
        ${X11_INCLUDE_DIRS}
        ${XCB_INCLUDE_DIRS}
    )
    target_link_libraries(wring PRIVATE
        ${X11_LIBRARIES}
        ${XCB_LIBRARIES}
    )
    target_compile_options(wring PRIVATE ${X11_CFLAGS_OTHER} ${XCB_CFLAGS_OTHER})
endif()

install(TARGETS wring RUNTIME DESTINATION bin)
