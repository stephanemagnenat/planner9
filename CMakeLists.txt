cmake_minimum_required(VERSION 2.6)

project(planner9)

find_package(Boost REQUIRED)

add_subdirectory(core)

add_subdirectory(simple)

add_subdirectory(threaded)

add_subdirectory(distributed)

add_subdirectory(programs)
