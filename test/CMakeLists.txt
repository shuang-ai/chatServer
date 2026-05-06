# ❌ 不要写 cmake_minimum_required（会重复）
# ❌ 不要重复设置 C++ 标准（会重复）
# ❌ 不要重复设置 muduo 路径（会重复）

# ✅ 只写子目录特有的内容

# 添加源文件
add_executable(muduo_server muduo_server.cpp)

# 链接库（会继承根目录的路径）
target_link_libraries(muduo_server
    muduo_net
    muduo_base
    pthread
)

# 如果子目录需要额外的头文件路径
# target_include_directories(muduo_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})