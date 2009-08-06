#ifndef RELATIONS_HPP_
#define RELATIONS_HPP_

#include "logic.hpp"
#include <cassert>
#include <map>


struct State;
struct Domain;
struct Atom;

struct AbstractFunction {
	virtual ~AbstractFunction() {}
	
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
	
	std::string name;
	size_t arity;
	
	virtual void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const {}
	virtual VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;
};

template<typename CoDomain>
struct Function : AbstractFunction {
	typedef CoDomain Storage;
	
	ScopedLookup<CoDomain> operator()(const char* first, ...);
	
	virtual CoDomain get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const CoDomain& value) const;
};

template<typename CoDomain>
struct SymmetricFunction : public Function<CoDomain> {

	SymmetricFunction(Domain* domain, const std::string& name);

	virtual CoDomain get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const CoDomain& value) const;
};

struct BooleanRelationStorage {
	operator bool () { return true; }
};

struct Relation: Function<bool> {
	typedef BooleanRelationStorage Storage;
	typedef ::ScopedProposition ScopedExpression;
	
	Relation(Domain* domain, const std::string& name, size_t arity);

	ScopedProposition operator()(const char* first, ...);
	
	virtual void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;
};

struct EquivalentRelation : public Relation {
	typedef BooleanRelationStorage Storage;
	typedef ::ScopedProposition ScopedExpression;

	EquivalentRelation(Domain* domain, const std::string& name);
	
	virtual bool get(const Variables& params, const State& state) const;
	virtual void set(const Variables& params, State& state, const bool& value) const;

	virtual void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	virtual VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;
};

struct EqualityRelation: public Relation {
	typedef BooleanRelationStorage Storage;
	typedef ::ScopedProposition ScopedExpression;

	EqualityRelation();

	bool get(const Atom& atom, const State& state) const;
	void set(const Literal& literal, State& state) const;
	void groundIfUnique(const Atom& atom, const State& state, const size_t constantsCount, Substitution& subst) const;
	VariablesRanges getRange(const Atom& atom, const State& state, const size_t constantsCount) const;
};
extern EqualityRelation equals;


#endif // RELATIONS_HPP_
