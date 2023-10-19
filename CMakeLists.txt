cmake_minimum_required(VERSION 3.15)
project(mysql_test)

#搜索当前目录下的所有.h .cpp .inl文件
file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.inl
)

set(MYSQL_DTR D:/caviar/AppData/MySQL/MySQLServer8.0)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${MYSQL_DTR}/include)

add_library(mysql_test STATIC ${SRC_FILES})

target_link_libraries(mysql_test ${MYSQL_DTR}/lib/libmysql.lib)
