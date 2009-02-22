# -------------------------------------------------
# Project created by QtCreator 2009-01-14T16:30:00
# -------------------------------------------------
QT -= gui
TARGET = planner9
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += src/domain.cpp \
    src/logic.cpp \
    src/plan.cpp \
    src/planner9.cpp \
    src/planner.cpp \
    src/problem.cpp \
    src/relations.cpp \
    src/scope.cpp \
    src/tasks.cpp
HEADERS += src/domain.hpp \
    src/logic.hpp \
    src/plan.hpp \
    src/planner9.hpp \
    src/problem.hpp \
    src/relations.hpp \
    src/scope.hpp \
    src/tasks.hpp \
    src/problems/robots.hpp \
    src/problems/basic.hpp
