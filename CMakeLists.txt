cmake_minimum_required(VERSION 4.2)
project(Snake CXX)

set(CMAKE_CXX_STANDARD 17)

# 1. 定义 raylib 路径
set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../raylib")
include_directories(${RAYLIB_DIR}/include)
link_directories(${RAYLIB_DIR}/lib)

# SQLite（新增功能）：头文件和导入库都在项目里的 SQLite 文件夹
set(SQLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SQLite")
include_directories(${SQLITE_DIR})

# 2. 共用文件：强制使用独显（双显卡笔记本上不加这个，raylib 窗口会全黑）
set(FORCE_GPU_FILE "F:/cpp/force_gpu.cpp")

# 3. 只能有一个 add_executable，这里我们统一叫 Snake
add_executable(Snake main.cpp ${FORCE_GPU_FILE})

# 4. 给 Snake 链接库（sqlite3 为新增）
target_link_libraries(Snake raylib opengl32 gdi32 winmm "${SQLITE_DIR}/libsqlite3.a")

# 5. 新增：运行时把 sqlite3.dll 拷到 exe 同目录
add_custom_command(TARGET Snake POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SQLITE_DIR}/sqlite3.dll" "$<TARGET_FILE_DIR:Snake>")