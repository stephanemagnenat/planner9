cmake_minimum_required(VERSION 2.6)

find_package(Qt4)
if (QT4_FOUND)
	set(QT_USE_QTDBUS TRUE)
	set(QT_USE_QTNETWORK TRUE)
	include(${QT_USE_FILE})
	add_definitions(${QT_DEFINITIONS})
	include_directories(${CMAKE_BINARY_DIR}/programs)

	add_executable(p9simple simple.cpp)
	target_link_libraries(p9simple planner9core)
	
	add_executable(p9simpleproba simple-proba.cpp)
	target_link_libraries(p9simpleproba planner9core)
	
	add_executable(p9threaded threaded.cpp)
	target_link_libraries(p9threaded planner9threaded planner9core ${Boost_LIBRARIES})

	qt4_automoc(distributed.cpp)
	add_executable(p9distributed distributed.cpp)
	target_link_libraries(p9distributed planner9distributed planner9core ${QT_LIBRARIES})
	
	add_executable(p9client distributed-client.cpp)
	target_link_libraries(p9client planner9distributed planner9core ${QT_LIBRARIES})
endif (QT4_FOUND)
