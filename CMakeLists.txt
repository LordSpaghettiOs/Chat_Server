cmake_minimum_required(VERSION 3.1)
project(Chat_Server C)

set(CMAKE_C_STANDARD 99)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(Chat_Server main.c client.c client.h server.c server.h)

target_link_libraries(Chat_Server Threads::Threads)