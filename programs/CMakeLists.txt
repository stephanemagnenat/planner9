add_executable(p9threaded threaded.cpp)
find_library(BOOST_THREAD_LIBRARY boost_thread ${Boost_LIBRARY_DIRS})
target_link_libraries(p9threaded planner9threaded planner9core ${BOOST_THREAD_LIBRARY})