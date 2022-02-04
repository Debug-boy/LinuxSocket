cmake_minimum_required(VERSION 3.21)
project(LinuxSocket)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp socket.hpp)
add_executable(client client.cpp socket.hpp)
