cmake_minimum_required(VERSION 3.20)
project(stunrun_native_smoke LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(stunrun-native-smoke smoke.cpp)
enable_testing()
add_test(NAME native-smoke COMMAND stunrun-native-smoke)
