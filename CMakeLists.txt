cmake_minimum_required(VERSION 2.6)

project(planner9)

find_package(Boost REQUIRED thread)

add_subdirectory(core)

add_subdirectory(threaded)

add_subdirectory(distributed)

add_subdirectory(programs)
