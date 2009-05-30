#ifndef SCOPE_HPP_
#define SCOPE_HPP_


#include "variable.hpp"
#include <string>


struct Scope {

	typedef std::string Name;
	typedef std::vector<Name> Names;

	Scope();
	Scope(const Name& name);
	Scope(const Names& names);

	Name getName(const Variable& variable) const;
	Variables getVariables(const Names& names) const;
	size_t getSize() const { return names.size(); }

	friend std::ostream& operator<<(std::ostream& os, const Scope& scope);

	Substitution merge(const Scope& that);

	Names names;

	class SetScope {
		SetScope(const Scope& scope);
		const Scope& scope;
		friend struct Scope;
		friend std::ostream& operator<<(std::ostream& os, const SetScope& indices);
		void set(std::ios& ios) const;
	};
	static SetScope setScope(const Scope& scope);
	static const Scope& getScope(std::ios& ios);

};


#endif // SCOPE_HPP_
