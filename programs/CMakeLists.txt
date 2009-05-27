cmake_minimum_required(VERSION 2.6)

add_executable(p9simple simple.cpp)
target_link_libraries(p9simple planner9core)

add_executable(p9threaded threaded.cpp)
find_library(BOOST_THREAD_LIBRARY boost_thread ${Boost_LIBRARY_DIRS})
target_link_libraries(p9threaded planner9threaded planner9core ${BOOST_THREAD_LIBRARY})
