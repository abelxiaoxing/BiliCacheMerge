cmake_minimum_required(VERSION 3.16)
project(BiliCacheMerge VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置Qt路径
set(CMAKE_PREFIX_PATH "/home/abelxiaoxing/Qt/6.9.2/gcc_64")

# 查找Qt6核心组件
find_package(Qt6 6.9.2 COMPONENTS
    Core
    Widgets
    Concurrent
    Network
    REQUIRED)

# 启用自动MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置编译器标志
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# 源文件列表
set(SOURCES
    src/main.cpp
    src/application/MainWindow.cpp
    src/application/ConfigDialog.cpp
    src/application/HelpDialog.cpp
    src/application/LogViewer.cpp
    src/application/PatternBuilderDialog.cpp
    src/core/ConfigManager.cpp
    src/core/FfmpegManager.cpp
    src/core/PatternManager.cpp
    src/core/FileScanner.cpp
    src/core/DanmakuConverter.cpp
    src/core/SubtitleDownloader.cpp
    src/core/CoverDownloader.cpp
    src/core/Utils.cpp
)

set(HEADERS
    src/application/MainWindow.h
    src/application/ConfigDialog.h
    src/application/HelpDialog.h
    src/application/LogViewer.h
    src/application/PatternBuilderDialog.h
    src/core/ConfigManager.h
    src/core/FfmpegManager.h
    src/core/PatternManager.h
    src/core/FileScanner.h
    src/core/DanmakuConverter.h
    src/core/SubtitleDownloader.h
    src/core/CoverDownloader.h
    src/core/Utils.h
)

# 资源文件
set(RESOURCES
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::Network
    m
)

# 设置应用程序属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/release"
)

# 设置应用程序图标 (Windows)
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE resources/app.rc)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)


# 输出构建信息
message(STATUS "BiliCacheMerge Qt C++ Build Configuration:")
message(STATUS "  Qt Version: 6.9.2")
message(STATUS "  C++ Standard: C++17")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin")