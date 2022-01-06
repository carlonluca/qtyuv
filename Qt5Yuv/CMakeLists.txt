#
# GPLv3 license
#
# Copyright (c) 2022 Luca Carlon
#
# This file is part of QtYuv
#
# QtYuv is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# QtYuv is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with QtYuv.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.14)

project(Qt5Yuv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick REQUIRED)

include(../deps/lqtutils/CMakeLists.txt)

set(PROJECT_SOURCES
    ${lqtutils_src}
    main.cpp
    yuvdisplay.cpp
    qml.qrc
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Qt5Yuv
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )
else()
    if(ANDROID)
        add_library(Qt5Yuv SHARED
            ${PROJECT_SOURCES}
            )
    else()
        add_executable(Qt5Yuv
            ${PROJECT_SOURCES}
            )
    endif()
endif()

target_include_directories(Qt5Yuv PRIVATE ../deps/lqtutils)
target_include_directories(Qt5Yuv PRIVATE ../deps/LightLogger)
target_compile_definitions(Qt5Yuv
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(Qt5Yuv
    PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick)

set_target_properties(Qt5Yuv PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(Qt5Yuv)
    qt_finalize_executable(Qt5Yuv)
endif()
