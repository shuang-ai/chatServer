cmake_minimum_required(VERSION 3.10)
project(chat)

# 全局设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件的搜索路径
# target_include_directories(${PROJECT_BINARY_DIR}/include)
# ① ${PROJECT_SOURCE_DIR}
# 这才是 → 你的项目根目录（就是放 CMakeLists.txt 的那个文件夹）
# ② ${PROJECT_BINARY_DIR}
# 执行的 cmake 命令的目录（通常就是 build 文件夹）
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/db)
include_directories(${PROJECT_SOURCE_DIR}/include/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

# 加载子目录(去src目录找cmake文件)
add_subdirectory(src)