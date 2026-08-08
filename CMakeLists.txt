cmake_minimum_required(VERSION 3.8)
project(chat)

#cpp 17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#编译期宏定义
add_definitions(-DOUTPUT_INFO)

# 配置最终的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/dataBase)
include_directories(${PROJECT_SOURCE_DIR}/thridParty)
include_directories(/home/cmsmn/project/mymuduo/coreFile)

#配置库文件搜索路径
link_directories(/usr/lib64/mysql)

# 加载子目录
add_subdirectory(src)
