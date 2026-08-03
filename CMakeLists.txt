cmake_minimum_required(VERSION 3.21)

project(IzotopepaQtGameV3 VERSION 3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(IzotopepaQtGameV3
    src/main.cpp
    src/gamewidget.cpp
    src/gamewidget.h
    src/world.cpp
    src/world.h
    src/entities.cpp
    src/entities.h
    src/enemybrain.cpp
    src/enemybrain.h
    src/drone.cpp
    src/drone.h
    src/turret.cpp
    src/turret.h
    src/enemyfactory.cpp
    src/enemyfactory.h
    src/level.cpp
    src/level.h
    src/jumppad.cpp
    src/jumppad.h
    src/pressureplate.cpp
    src/pressureplate.h
    src/conveyor.cpp
    src/conveyor.h
    src/onewayplatform.cpp
    src/onewayplatform.h
    src/fallingplatform.cpp
    src/fallingplatform.h
    src/environmentvolume.cpp
    src/environmentvolume.h
    src/worldevent.cpp
    src/worldevent.h
    src/support.cpp
    src/support.h
    resources.qrc
)

target_link_libraries(
    IzotopepaQtGameV3
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

add_executable(IzotopepaLevelEditor
    editor/main.cpp
    editor/editormainwindow.cpp
    editor/editormainwindow.h
    editor/levelcanvas.cpp
    editor/levelcanvas.h
    editor/leveldocument.cpp
    editor/leveldocument.h
)

target_link_libraries(
    IzotopepaLevelEditor
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)

if(WIN32)
    set_target_properties(
        IzotopepaQtGameV3
        IzotopepaLevelEditor
        PROPERTIES WIN32_EXECUTABLE TRUE
    )
endif()
