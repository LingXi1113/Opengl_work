cmake_minimum_required(VERSION 3.10)
project(NewScene)

add_executable(new_scene main.cpp)

target_link_libraries(new_scene PRIVATE glad glfw stb_image learnopengl)
