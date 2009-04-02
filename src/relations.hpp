#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_


#include "logic.hpp"
#include <cassert>
#include <map>


struct State;


struct Relation {

	// TODO: move this elsewhere?
	struct VariableRange:public std::vector<bool> {
		VariableRange() {}
		VariableRange(size_type n, const bool value	= false) : std::vector<bool>(n, value) { }
		void operator |=(const VariableRange& that) {
			assert(size() == that.size());
			for (size_t i = 0; i < size(); ++i)
				(*this)[i] = (*this)[i] || that[i];
		}
		void operator &=(const VariableRange& that) {
			assert(size() == that.size());
			for (size_t i = 0; i < size(); ++i)
				(*this)[i] = (*this)[i] && that[i];
		}
		void operator ~(void) {
			for (size_t i = 0; i < size(); ++i)
				(*this)[i] = ~ (*this)[i];
		}
		bool isEmpty() const {
			bool anyTrueFound = false;
			for (const_iterator it = begin(); it != end(); ++it) {
				anyTrueFound = anyTrueFound || (*it);
			}
			return anyTrueFound == false;
		}
		friend std::ostream& operator<<(std::ostream& os, const VariableRange& range) {
			os << "(";
			for (const_iterator it = range.begin(); it != range.end(); ++it) {
				os << *it;
				if (it + 1 != range.end())
					os << ",";
			}
			os << ")";
			return os;
		}
	};
	typedef std::map<Variable, VariableRange> VariablesRanges;

	Relation(const std::string& name, size_t arity);

	ScopedProposition operator()(const char* first, ...);

	virtual bool check(const Atom& atom, const State& state) const;
	virtual void set(const Literal& literal, State& state) const;
	virtual void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;

	std::string name;
	size_t arity;

};

struct EquivalentRelation : public Relation {

	EquivalentRelation(const std::string& name);

	bool check(const Atom& atom, const State& state) const;
	void set(const Literal& literal, State& state) const;
	void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;

protected:
	Atom createAtom(const Variable& p0, const Variable& p1) const;
};

struct EqualityRelation: public Relation {

	EqualityRelation();

	bool check(const Atom& atom, const State& state) const;
	void set(const Literal& literal, State& state) const;
	void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;
};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
