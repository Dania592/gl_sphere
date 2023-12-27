cmake_minimum_required(VERSION 3.23)
project(gl_sphere C)

set(CMAKE_C_STANDARD 11)

add_executable(gl_sphere Espace_para/espace_param.c)
target_link_libraries(gl_sphere opengl32 glu32 freeglut)
