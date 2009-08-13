cmake_minimum_required(VERSION 2.6)

find_package(Qt4)
if (QT4_FOUND)
	include(${QT_USE_FILE})
	include_directories (${QT_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/programs)

	add_executable(p9simple simple.cpp)
	target_link_libraries(p9simple planner9core ${QT_QTCORE_LIBRARIES})
	
	add_executable(p9threaded threaded.cpp)
	target_link_libraries(p9threaded planner9threaded planner9core ${Boost_THREAD_LIBRARY})

	qt4_automoc(distributed.cpp)
	add_executable(p9distributed distributed.cpp)
	target_link_libraries(p9distributed ${QT_QTCORE_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTDBUS_LIBRARIES} planner9distributed planner9core)
	
	add_executable(p9client distributed-client.cpp)
	target_link_libraries(p9client ${QT_QTCORE_LIBRARIES} ${QT_QTNETWORK_LIBRARIES} ${QT_QTDBUS_LIBRARIES} planner9distributed planner9core)
endif (QT4_FOUND)
