#include "relations.hpp"
#include <cstdarg>


Relation::Relation(const std::string& name, size_t arity):
	name(name),
	arity(arity) {
}

ScopedProposition Relation::operator()(const char* first, ...) {
	Scope::Names names;
	names.push_back(first);
	va_list vargs;
	va_start(vargs, first);
	for (size_t i = 1; i < arity; ++i)
		names.push_back(va_arg(vargs, const char*));
	va_end(vargs);

	Scope scope(names);
	Scope::Indices indices = scope.getIndices(names);

	std::auto_ptr<const Proposition> atom(new Atom(this, indices));
	return ScopedProposition(scope, atom);
}


EquivalentRelation::EquivalentRelation(const std::string& name):
	Relation(name, 2) {
}


EqualityRelation::EqualityRelation():
	Relation("=", 2) {
}

EqualityRelation equals;
