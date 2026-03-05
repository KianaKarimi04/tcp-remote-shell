cmake_minimum_required(VERSION 3.10)

project(remote_shell C)

set(CMAKE_C_STANDARD 17)

add_executable(server server.c)
add_executable(client client.c)