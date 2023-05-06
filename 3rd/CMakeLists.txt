cmake_minimum_required(VERSION 3.24)
project(3rd)

set(SDL_SHARED false)
set(SDL_STATIC true)
set(SDL_TEST false)
set(SDL_TEST_ENABLED_BY_DEFAULT OFF)
set(BUILD_SHARED_LIBS OFF)
set(SDL3IMAGE_BUILD_SHARED_LIBS false)

add_subdirectory(glm)