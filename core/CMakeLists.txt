set (PLANNER9CORE_SRC
	domain.cpp
	logic.cpp
	expressions.cpp
	state.cpp
	plan.cpp
	planner9.cpp
	problem.cpp
	relations.cpp
	scope.cpp
	tasks.cpp
	variable.cpp
	costs.cpp
)

add_library(planner9core ${PLANNER9CORE_SRC})
