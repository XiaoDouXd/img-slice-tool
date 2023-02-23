cmake_minimum_required(VERSION 3.25)
project(app)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

file(GLOB_RECURSE ALL_SOURCE "src/*.cpp" "src/*.c" "src/*.h" "src/*.hpp" "../3rd/rapidxml/*.hpp")

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
include_directories(${PROJECT_NAME} ./src ./src/base ../3rd/rapidxml ../3rd/stb)
add_executable(${PROJECT_NAME} ${ALL_SOURCE})
if(WIN32)
    file(GLOB_RECURSE ALL_RC "../appBase/core/rc/gen/rc/*.rc")
    target_sources(${PROJECT_NAME} PRIVATE ${ALL_RC})
endif(WIN32)

# 控制台仅在 Debug 模式下生成
if (${CMAKE_BUILD_TYPE} MATCHES ".*(D|d)(E|e)(B|b)(U|u)(G|g).*")
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译带有控制台的目标程序")
else()
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译隐藏控制台的目标程序")
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()

# 链接 appBase
target_include_directories(${PROJECT_NAME} PRIVATE ../appBase/core/include)
target_link_libraries(${PROJECT_NAME} PRIVATE appBase)

# 链接 glm
target_include_directories(${PROJECT_NAME} PRIVATE ../3rd/glm/glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)