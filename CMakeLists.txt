cmake_minimum_required(VERSION 3.10...3.27)

project(GridMapEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Qt6的路径
set(CMAKE_PREFIX_PATH "G:/Qt/6.5.3/mingw_64")

# 查找Qt组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 添加头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/grideditor.cpp
    src/codeeditor.cpp
    src/codehighlighter.cpp
    src/gridcreatedialog.cpp
    src/examplecodedialog.cpp
    src/pathfindingexecutor.cpp
    src/randomobstacledialog.cpp
    include/mainwindow.h
    include/grideditor.h
    include/codehighlighter.h
    include/gridcreatedialog.h
    include/examplecodedialog.h
    include/pathfindingexecutor.h
    include/randomobstacledialog.h
    resources.qrc
    app.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(GridMapEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(GridMapEditor
        ${PROJECT_SOURCES}
    )
endif()

target_include_directories(GridMapEditor PRIVATE include)

target_link_libraries(GridMapEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 设置应用程序属性
set_target_properties(GridMapEditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# 禁用Vulkan支持，因为我们不需要它
set(QT_NO_CREATE_VERSIONLESS_TARGETS ON)
set(VULKAN_FOUND FALSE)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(GridMapEditor)
endif() 