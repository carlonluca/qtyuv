cmake_minimum_required(VERSION 3.16)

project(QtYuv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 5.15.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appQtYuv
    main.cpp
    yuvmaterial.cpp
)

qt_add_qml_module(appQtYuv
    URI QtYuv
    VERSION 1.0
    QML_FILES main.qml 
)

target_compile_definitions(appQtYuv
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appQtYuv
    PRIVATE Qt6::Quick)
