cmake_minimum_required(VERSION 3.4)

#项目名称
project(VolumeRenderer)

#项目选项
option(BUILD_RELEASE "build release excutable" ON)
option(BUILD_SERVER "build server" ON)
option(BUILD_CLIENT "build client" ON)

#设置可执行程序的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

set(MAC ON)
set(BUILD_RELEASE ON)

#设置C++编译选项
add_subdirectory(server)
#add_subdirectory(client)

#if (BUILD_SERVER)
#    add_subdirectory(server)
#endif ()
#if (BUILD_CLIENT)
#    add_subdirectory(client)
#endif ()
