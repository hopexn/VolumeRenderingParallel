cmake_minimum_required(VERSION 3.4)

#项目名称
project(VolumeRendererClient)

#设置C++编译选项
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
set(CMAKE_BUILD_TYPE Release)

#添加Qt5依赖
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets REQUIRED)
add_definitions(${QT_DEFINITIONS})

#使用boost
find_package(Boost REQUIRED)

include_directories(../common)

#将volume和ui文件夹下的源码放入源码列表中
aux_source_directory(ui UI_SRC_LIST)

#生成可执行程序
add_executable(${PROJECT_NAME} main.cpp ${UI_SRC_LIST} network/Message.h)

#链接QT
target_link_libraries(${PROJECT_NAME} Qt5::Widgets boost_timer boost_system)