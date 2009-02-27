#ifndef SCOPE_HPP_
#define SCOPE_HPP_


#include <iostream>
#include <string>
#include <utility>
#include <vector>


struct Scope {

	typedef std::string Name;
	typedef std::vector<Name> Names;

	typedef size_t Index;
	struct Indices: std::vector<Index> {
		void substitute(const Indices& subst);
		Indices substitute(const Indices& subst) const;
		friend std::ostream& operator<<(std::ostream& os, const Indices& indices);
	};

	Scope();
	Scope(const Name& name);
	Scope(const Names& names);

	Name getName(Index index) const;
	Names getNames(const Indices& indices) const;
	Indices getIndices(const Names& names) const;
	size_t getSize() const { return names.size(); }
	
	friend std::ostream& operator<<(std::ostream& os, const Scope& scope);

	typedef std::pair<Indices, Indices> Substitutions;
	Substitutions merge(const Scope& that);

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

private:

	Substitutions merge(const Names& names2);

};


#endif // SCOPE_HPP_
