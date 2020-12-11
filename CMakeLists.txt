cmake_minimum_required(VERSION 3.5)
project(BeiYueManagementSystem)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_FILES main.cpp C++/FeedbackManage.cpp C++/UserManage.cpp C++/WordManage.cpp)
add_executable(BeiYueManagementSystem ${SOURCE_FILES})
