cmake_minimum_required(VERSION 3.25)
project(ImgClipTool)        #------ 20230207 切图器

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# --------------------------------- 核心
include(./Config.cmake)
add_subdirectory(app)       #------ gui 依赖 vulkan-sdl-imgui
add_subdirectory(appBase)

# --------------------------------- 第三方库
add_subdirectory(3rd)