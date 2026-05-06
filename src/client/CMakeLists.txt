aux_source_directory(. SRC_LIST)

add_executable(ChatClient ${SRC_LIST})
# 指定可执行文件的依赖库
target_link_libraries(ChatClient pthread)