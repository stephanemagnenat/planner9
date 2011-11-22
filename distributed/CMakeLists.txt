find_package(Qt4)

if (QT4_FOUND)
	set(QT_USE_QTDBUS TRUE)
	set(QT_USE_QTNETWORK TRUE)
	include(${QT_USE_FILE})
	add_definitions(${QT_DEFINITIONS})
	
	set (PLANNER9DISTRIBUTED_SRC
		avahi-server.cpp
		avahi-entry-group.cpp
		avahi-service-browser.cpp
		serializer.cpp
		chunked.cpp
		planner9-distributed.cpp
		planner9-dbus.cpp
	)
	
	qt4_automoc(${PLANNER9DISTRIBUTED_SRC})
	
	include_directories (${CMAKE_BINARY_DIR}/distributed)
	
	add_library(planner9distributed ${PLANNER9DISTRIBUTED_SRC})
endif (QT4_FOUND)
