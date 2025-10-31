cmake_minimum_required(VERSION 3.16)

project(qt_bili_merge
    VERSION 1.0.0
    DESCRIPTION "Qt B站缓存合并工具"
    LANGUAGES CXX
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# 可选的LinguistTools组件
find_package(Qt6 COMPONENTS LinguistTools QUIET)

# 设置包含目录
include_directories(include)
include_directories(src)

# 收集源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# 确保所有源文件都被包含
message(STATUS "Source files found:")
foreach(source ${SOURCES})
    message(STATUS "  ${source}")
endforeach()

# 收集资源文件
set(RESOURCES
    resources/resources.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release
)

# 复制FFmpeg二进制文件
if(WIN32)
    file(GLOB FFMPEG_FILES "resources/ffmpeg/*.exe")
else()
    file(GLOB FFMPEG_FILES "resources/ffmpeg/ffmpeg*")
endif()

foreach(FFMPEG_FILE ${FFMPEG_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FFMPEG_FILE}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endforeach()

# 复制Qt运行时库（Windows）
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
        )
    endif()
endif()

# 设置调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)
endif()

# 编译器特定设置
if(MSVC)
    # Windows特定设置
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
else()
    # Linux/Mac特定设置
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 复制资源文件安装
install(DIRECTORY resources/
    DESTINATION share/${PROJECT_NAME}/resources
    FILES_MATCHING PATTERN "*.qrc" PATTERN "*.png" PATTERN "*.jpg" PATTERN "*.ico"
)

install(DIRECTORY config/
    DESTINATION share/${PROJECT_NAME}/config
)