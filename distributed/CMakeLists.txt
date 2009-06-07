find_package(Qt4)

if (QT4_FOUND)
	include(${QT_USE_FILE})
	
	set (PLANNER9DISTRIBUTED_SRC
		avahi-server.cpp
		avahi-entry-group.cpp
		avahi-service-browser.cpp
		serializer.cpp
		chunked.cpp
		planner9-distributed.cpp
	)
	
	qt4_automoc(${PLANNER9DISTRIBUTED_SRC})
	
	include_directories (${QT_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${CMAKE_BINARY_DIR}/distributed)
	
	add_library(planner9distributed ${PLANNER9DISTRIBUTED_SRC})
endif (QT4_FOUND)
