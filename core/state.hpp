#ifndef STATE_HPP_
#define STATE_HPP_


#include "logic.hpp"
#include <algorithm>
#include <set>
#include <boost/cast.hpp>
#include <istream>
#include <sstream>

struct AbstractFunction;
struct Serializer;

struct State {
	struct AbstractFunctionState {
		virtual ~AbstractFunctionState() {}
		
		virtual AbstractFunctionState* clone() const = 0;
		
		virtual void insert(const Variables& params, const std::string& value) = 0;
		
		virtual void dump(std::ostream& os, bool& first, const std::string& functionName) = 0;
		
		virtual void serialize(Serializer& serializer) const = 0;
		virtual void deserialize(Serializer& serializer, size_t arity) = 0;		
	};
	
	template <typename T>
	static T GetDefaultInsertValue() {
		return T();		
	}
	
	template<typename ValueType>
	struct FunctionState: AbstractFunctionState {
		typedef std::map<Variables, ValueType> Values;
		Values values;
		
		virtual AbstractFunctionState* clone() const {
			return new FunctionState<ValueType>(*this);
		}
		
		virtual void insert(const Variables& params, const std::string& value) {
			if (value.empty()) {
				ValueType v;
				std::istringstream iss(value);
				iss >> v;
				values[params] = v;
			} else {
				values[params] = GetDefaultInsertValue<ValueType>();
			}
		}
		
		virtual void dump(std::ostream& os, bool& first, const std::string& functionName) {
			for (typename Values::const_iterator it = values.begin(); it != values.end(); ++it) {
				if(first) {
					first = false;
				} else {
					os << ", ";
				}
				os << functionName << "(" << it->first << ") : " << it->second;
			}
		}
		
		// do not call these function unless you implement them;
		// the weak attribute will prevent a compilation error but will result in a runtime crash.
		__attribute__ ((weak)) virtual void serialize(Serializer& serializer) const;
		__attribute__ ((weak)) virtual void deserialize(Serializer& serializer, size_t arity);
	};
	
	typedef std::map<const AbstractFunction*, AbstractFunctionState*> Functions;
	typedef std::pair<const AbstractFunction*, AbstractFunctionState*> FunctionsEntry;
	Functions functions;
	
	State();
	State(const State& that);
	~State();
	
	// params must be in global scope
	template<typename ValueType>
	void insert(const AbstractFunction* function, const Variables& params, const ValueType& value) {
		typedef FunctionState<ValueType> FunctionState;
		
		Functions::iterator it(functions.find(function));
		if (it == functions.end()) {
			it = functions.insert(FunctionsEntry(function,new FunctionState())).first;
		}
		
		FunctionState* functionState(boost::polymorphic_downcast<FunctionState*>(it->second));
		functionState->values[params] = value;
	}
	
	template<typename ValueType>
	void erase(const AbstractFunction* function, const Variables& params) {
		typedef FunctionState<ValueType> FunctionState;
		
		Functions::iterator it = functions.find(function);
		if (it == functions.end())
			return;
		FunctionState* functionState(boost::polymorphic_downcast<FunctionState*>(it->second));
		functionState->values.erase(params);
		if (functionState->values.empty())	{
			delete it->second;
			functions.erase(it);
		}
	}
	
	friend std::ostream& operator<<(std::ostream& os, const State& state);
};

template <>
inline bool State::GetDefaultInsertValue<bool>() {
	return true;
}


std::ostream& operator<<(std::ostream& os, const State& state);


#endif // STATE_HPP_
