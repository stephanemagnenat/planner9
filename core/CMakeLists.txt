set (PLANNER9CORE_SRC
	domain.cpp
	logic.cpp
	plan.cpp
	planner9.cpp
	problem.cpp
	relations.cpp
	scope.cpp
	tasks.cpp
)

add_library(planner9core ${PLANNER9CORE_SRC})