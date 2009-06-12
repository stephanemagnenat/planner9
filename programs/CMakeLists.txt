cmake_minimum_required(VERSION 2.6)

add_executable(p9simple simple.cpp)
target_link_libraries(p9simple planner9core)

add_executable(p9threaded threaded.cpp)
find_library(BOOST_THREAD_LIBRARY boost_thread ${Boost_LIBRARY_DIRS})
target_link_libraries(p9threaded planner9threaded planner9core ${BOOST_THREAD_LIBRARY})

find_package(Qt4)
if (QT4_FOUND)
	include(${QT_USE_FILE})
	include_directories (${QT_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/programs)
	qt4_automoc(distributed.cpp)
	add_executable(p9distributed distributed.cpp)
	target_link_libraries(p9distributed ${QT_QTCORE_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTDBUS_LIBRARIES} planner9distributed planner9core)
endif (QT4_FOUND)
