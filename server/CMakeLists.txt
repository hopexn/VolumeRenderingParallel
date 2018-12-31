cmake_minimum_required(VERSION 3.4)

#项目名称
project(VolumeRendererServer)

#设置C++编译选项
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpic++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_BUILD_TYPE Release)

#将volume和ui文件夹下的源码放入源码列表中
set(COMMON_SRCS ../common/BBox.hpp ../common/Camera.hpp ../common/Image.hpp ../common/Vector.hpp
        ../common/util.h ../common/avx_func.h)

set(VOLUME_SRCS volume/TF1D.hpp volume/Block.hpp volume/MMU.hpp volume/Volume.hpp volume/Renderer.hpp ../common/server_common.h)

link_directories(../libs)

add_executable("${PROJECT_NAME}_test" test.cpp ${COMMON_SRCS} )
target_link_libraries("${PROJECT_NAME}_test" boost_system libjpeg.a)

add_executable("${PROJECT_NAME}_master" master.cpp ${COMMON_SRCS})
target_link_libraries("${PROJECT_NAME}_master" boost_system libjpeg.a)

add_executable("proxy" proxy.cpp ${COMMON_SRCS})
target_link_libraries("proxy" boost_system)

#生成可执行程序
add_executable("${PROJECT_NAME}_slave" slave.cpp ${VOLUME_SRCS} ${COMMON_SRCS})
target_link_libraries("${PROJECT_NAME}_slave" boost_system)