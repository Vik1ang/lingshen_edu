# 单个目录实现
# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 工程，他不是执行文件名
PROJECT(0VOICE)
# 手动加入文件 ${变量名}} ，比如${SRC_LIST}
SET(SRC_LIST main.c)
SET(SRC_LIST2 main2.c)
# MESSAGE和echo类似
MESSAGE(STATUS "THIS IS BINARY DIR " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "THIS IS SOURCE DIR " ${PROJECT_SOURCE_DIR})

# 生产执行文件名0voice  0voice2
ADD_EXECUTABLE(0voice ${SRC_LIST})
ADD_EXECUTABLE(0voice2 ${SRC_LIST2})
