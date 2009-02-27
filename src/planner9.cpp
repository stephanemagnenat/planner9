#include "planner9.hpp"
#include "domain.hpp"
#include "problem.hpp"
#include "logic.hpp"
#include "relations.hpp"
#include <iostream>
#include <set>


// nodes in our search tree
struct TreeNode {
	TreeNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	const Plan plan;
	const TaskNetwork network; // T
	const size_t allocatedVariablesCount;
	const Cost cost;
	const CNF preconditions;
	const State state;
};

TreeNode::TreeNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state):
	plan(plan),
	network(network),
	allocatedVariablesCount(allocatedVariablesCount),
	cost(cost),
	preconditions(preconditions),
	state(state) {
}


Planner9::Planner9(const Problem& problem):
	problem(problem),
	iterationCount(0) {
}

// HTN: procedure SHOP2(s, T, D)
boost::optional<Plan> Planner9::plan() {
	// HTN: P = the empty plan
	addNode(Plan(), problem.network, problem.scope.getSize(), 0, CNF(), problem.state);

	/* TODO: prevent infinite recursion with either:
	 * - maximum weight
	 * - maximum nodes
	 * - maximum depth
	 * - maximum time
	 */
	// HTN: loop
	while(!nodes.empty() && plans.empty()) {
		step();
	}
	std::cout << "Terminated after " << iterationCount << " iterations" << std::endl;

	if(plans.empty())
		return false;
	else
		return plans.front();
}

void Planner9::step() {
	Nodes::iterator front = nodes.begin();
	Cost cost = front->first;
	TreeNode* node = front->second;

	std::cout << "\n-" << cost << std::endl;
	std::cout << "do " << node->network << std::endl;
	std::cout << "such that " << node->preconditions << std::endl;
	std::cout << "knowing " << node->state << std::endl;

	nodes.erase(front);

	visitNode(node->plan, node->network, node->allocatedVariablesCount, node->cost, node->preconditions, node->state);

	delete node;
}

void Planner9::visitNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state) {
	++iterationCount;

	// HTN: T0 ← {t ∈ T : no other task in T is constrained to precede t}
	const Tasks& t0 = network.first;
	// HTN: if T = ∅ then return P
	if (t0.empty()) {
		std::cout << preconditions << std::endl;
		success(plan);
	}

	// HTN: nondeterministically choose any t ∈ T0
	for (Tasks::const_iterator taskIt = t0.begin(); taskIt != t0.end(); ++taskIt)
	{
		const Task* t = *taskIt;
		const Head* head = t->head;

		const Action* action = dynamic_cast<const Action*>(head);
		if(action != 0) {
			std::cout << "action\n";
			// HTN: if t is a primitive task then

			// TODO: HTN: A ← {(a, θ) : a is a ground instance of an operator in D, θ is a substi-
			// HTN: tution that uniﬁes {head(a), t}, and s satisﬁes a’s preconditions}
			// HTN: if A = ∅ then return failure
			// HTN: nondeterministically choose a pair (a, θ) ∈ A
			// TODO: HTN: modify s by deleting del(a) and adding add(a)

			TaskNetwork newNetwork = network.erase(taskIt);

			Scope::Indices subst = t->getSubstitution(action->getVariables(), allocatedVariablesCount);
			size_t newAllocatedVariablesCount = allocatedVariablesCount + action->getScope().getSize() - head->getParamsCount();

			CNF newPreconditions(action->getPrecondition());
			newPreconditions.substitute(subst);
			newPreconditions += preconditions;

			// collect all variables and relation affected by the effects
			typedef std::set<const Relation*> AffectedRelations;
			typedef Relation::VariableRange VariableRange;
			typedef Relation::VariablesRange VariablesRange;
			// TODO: maybe a vector of pairs is better
			typedef std::map<Scope::Index, VariableRange> AffectedVariables;

			// first iterate on all effects
			Action::Effects effects(action->getEffects());
			AffectedRelations affectedRelations;
			AffectedVariables affectedVariables;
			for(Action::Effects::iterator it = effects.begin(); it != effects.end(); ++it) {
				Literal& literal = *it;
				literal.substitute(subst);
				affectedRelations.insert(literal.atom.relation);
				for (Scope::Indices::const_iterator jt = literal.atom.params.begin(); jt != literal.atom.params.end(); ++jt) {
					const Scope::Index index = *jt;
					if(index >= problem.scope.getSize())
						affectedVariables[index].resize(problem.scope.getSize(), false);
				}
			}

			// then look into preconditions for all indirectly affected variables
			std::cout << "pre " << newPreconditions << std::endl;
			for(CNF::iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
				for(CNF::Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
					const Atom& atom = jt->atom;
					if (affectedRelations.find(atom.relation) != affectedRelations.end()) {
						// the relation of this atom is affected, all its variables must be grounded
						for (Scope::Indices::const_iterator kt = atom.params.begin(); kt != atom.params.end(); ++kt) {
							const Scope::Index index = *kt;
							if(index >= problem.scope.getSize())
								affectedVariables[index].resize(problem.scope.getSize(), false);
						}
					}
				}
			}
			
			// create a list of affected variables along with their ranges, set their range to maximum
			AffectedVariables filteredAffectedVariables(affectedVariables);
			for (AffectedVariables::iterator varIt = filteredAffectedVariables.begin(); varIt != filteredAffectedVariables.end(); ++varIt) {
				std::fill(varIt->second.begin(), varIt->second.end(), true);
			}
				
			// filter out variables ranges using state
			for(CNF::iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
				AffectedVariables inDisjunctionRanges;
				for(CNF::Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
					const Literal& literal = *jt;
					const Atom& atom = literal.atom;
					VariablesRange variablesRange(atom.params.size(), VariableRange(problem.scope.getSize(), false));
					atom.relation->getRange(state, variablesRange);
					// invert ranges if negated
					if (literal.negated) {
						for (VariablesRange::iterator kt = variablesRange.begin(); kt != variablesRange.end(); ++kt) {
							VariableRange& range = *kt;
							~range;
						}
					}
					// extend range of variables if it is to be grounded
					for (size_t kt = 0; kt != atom.params.size(); ++kt) {
						const Scope::Index& index = atom.params[kt];
						// TODO: optimize this with an index check
						if (affectedVariables.find(index) != affectedVariables.end()) {
							AffectedVariables::iterator affectedIt = inDisjunctionRanges.find(index);
							if (affectedIt != inDisjunctionRanges.end()) {
								VariableRange& range = affectedIt->second;
								range |= variablesRange[kt];
							} else {
								inDisjunctionRanges[index] = variablesRange[kt];
							}
						}
					}
				}
				/*for (AffectedVariables::iterator varIt = inDisjunctionRanges.begin(); varIt != inDisjunctionRanges.end(); ++varIt) {
					std::cerr << varIt->first << ": " << varIt->second << std::endl;
				}*/
				
				// filtered global ranges with ranges of this disjunction
				for (AffectedVariables::iterator varIt = inDisjunctionRanges.begin(); varIt != inDisjunctionRanges.end(); ++varIt) {
					AffectedVariables::iterator filteredVarIt = filteredAffectedVariables.find(varIt->first);
					assert(filteredVarIt != filteredAffectedVariables.end());
					filteredVarIt->second &= varIt->second;
				}
			}
			// copy back (only for easier debugging, remove afterwards)
			affectedVariables = filteredAffectedVariables;
			
			std::cerr << "constants " << problem.scope << std::endl;

			std::cerr << "assigning";
			for(AffectedVariables::const_iterator it = affectedVariables.begin(); it != affectedVariables.end(); ++it) {
				std::cerr << " var" << it->first - problem.scope.getSize() << " " << it->second ;
			}
			std::cerr << std::endl;
			
			// if any of the variable has no domain, return
			for(AffectedVariables::const_iterator it = affectedVariables.begin(); it != affectedVariables.end(); ++it) {
				if (it->second.isEmpty())
					return;
			}

			// ground variables
			typedef std::map<Scope::Index, Scope::Index> VariablesAssignment;
			typedef std::vector<VariablesAssignment> VariablesAssignments;
			VariablesAssignments allAssignments;
			allAssignments.push_back(VariablesAssignment());
			for(AffectedVariables::const_iterator it = affectedVariables.begin(); it != affectedVariables.end(); ++it) {
				VariablesAssignments newAssignments;
				const AffectedVariables::value_type& variable = *it;
				const VariableRange& range = variable.second;
				for (VariableRange::const_iterator kt = range.begin(); kt != range.end(); ++kt)	{
					if(*kt) {
						for (VariablesAssignments::const_iterator jt = allAssignments.begin(); jt != allAssignments.end(); ++jt) {
							VariablesAssignment assignment = *jt;
							assignment[variable.first] = kt - range.begin();
							newAssignments.push_back(assignment);
						}
					}
				}
				std::swap(allAssignments, newAssignments);
			}

			// iterate on all ground assignments and create corresponding nodes
			for(VariablesAssignments::const_iterator it = allAssignments.begin(); it != allAssignments.end(); ++it) {
				Scope::Indices subst;
				VariablesAssignment::const_iterator jt = it->begin();
				size_t assignedVariablesCount = 0;
				for(Scope::Index i = 0; i < newAllocatedVariablesCount; ++i) {
					if (jt == it->end() || i != jt->first) {
						subst.push_back(i - assignedVariablesCount);
					} else {
						subst.push_back(jt->second);
						++jt;
						++assignedVariablesCount;
					}
				}

				// HTN: modify T by removing t and applying θ

				CNF assignedPreconditions(newPreconditions);
				assignedPreconditions.substitute(subst);

				// Check what can be checked in these preconditions
				CNF remainingPreconditions;
				bool satisfiable = true;
				for(CNF::const_iterator it = assignedPreconditions.begin(); it != assignedPreconditions.end(); ++it) {
					bool tautology = false;
					CNF::Disjunction disjunction;
					for(CNF::Disjunction::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
						const Literal& literal = *jt;
						const Atom& atom = literal.atom;
						const Scope::Indices& params = atom.params;
						// TODO: ensure everywhere that precond do not contain fully grounded atoms and optimize out this loop
						bool grounded = true;
						for(Scope::Indices::const_iterator kt = params.begin(); kt != params.end(); ++kt) {
							if(*kt >= problem.scope.getSize()) {
								grounded = false;
								break;
							}
						}
						if(grounded) {
							const bool isTrue = atom.relation->check(atom, state) ^ literal.negated;
							if(isTrue) {
								// the literal is true => the clause is true
								tautology = true;
								break;
							} else {
								// the literal is false => skip it
							}
						} else {
							// there are still free variables in this literal
							disjunction.push_back(literal);
						}
					}
					if(!tautology) {
						if(!disjunction.empty()) {
							remainingPreconditions.push_back(disjunction);
						} else {
							// the clause is unsatisfiable => the conjunction is unsatisfiable
							satisfiable = false;
							break;
						}
					}
				}

				if(satisfiable) {
					TaskNetwork assignedNetwork(newNetwork.substitute(subst));
					size_t assignedAllocatedVariablesCount = newAllocatedVariablesCount - assignedVariablesCount;

					// HTN: append a to P
					Plan p(plan);
					p.push_back(t->substitute(subst));

					// apply effects
					const State newState = effects.apply(state, subst);

					// HTN: T0 ← {t ∈ T : no task in T is constrained to precede t}
					addNode(p, assignedNetwork, assignedAllocatedVariablesCount, cost, remainingPreconditions, newState);
				}
			}
		}

		// if t is a method then decompose
		const Method* method = dynamic_cast<const Method*>(head);
		if (method != 0) {
			std::cout << "method\n";
			// HTN: else

			// HTN: M ← {(m, θ) : m is an instance of a method in D, θ uniﬁes {head(m), t},
			// HTN: 			   pre(m) is true in s, and m and θ are as general as possible}
			// HTN: if M = ∅ then return failure
			// HTN: nondeterministically choose a pair (m, θ) ∈ M

			// push all alternatives
			for (Method::Alternatives::const_iterator altIt = method->alternatives.begin(); altIt != method->alternatives.end(); ++altIt) {
				const Method::Alternative& alternative = *altIt;

				Scope::Indices subst = t->getSubstitution(alternative.variables, allocatedVariablesCount);
				size_t newAllocatedVariablesCount = allocatedVariablesCount + alternative.scope.getSize() - head->getParamsCount();

				CNF newPreconditions(alternative.precondition);
				newPreconditions.substitute(subst);
				std::cout << "raw pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;
				if (newPreconditions.simplify(state, problem.scope.getSize()) == true) {
					std::cout << "simp. pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;
					newPreconditions += preconditions;
	
					//std::cout << "alt: " << Scope::setScope(alternative.scope) << alternative.tasks;
					//std::cout << "pb:  " << Scope::setScope(problem.scope) << alternative.tasks.substitute(subst);
					//std::cout << std::endl;
	
					// HTN: modify T by removing t, adding sub(m), constraining each task
					// HTN: in sub(m) to precede the tasks that t preceded, and applying θ
					TaskNetwork newNetwork = network.replace(taskIt, alternative.tasks.substitute(subst));
	
					Cost newCost = cost + alternative.cost;
	
					// HTN: if sub(m) = ∅ then
					// HTN: T0 ← {t ∈ sub(m) : no task in T is constrained to precede t}
					// HTN: else T0 ← {t ∈ T : no task in T is constrained to precede t}
					addNode(plan, newNetwork, newAllocatedVariablesCount, newCost, newPreconditions, state);
				} else {
					std::cout << "simp. pre failed" << std::endl;
				}
			}
		}
	}
}

void Planner9::addNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state) {
	TreeNode* node = new TreeNode(plan, network, allocatedVariablesCount, cost, preconditions, state);

	cost += network.getSize();
	std::cout << "+" << cost << std::endl;
	std::cout << "do " << node->network << std::endl;
	std::cout << "such that " << node->preconditions << std::endl;
	std::cout << "knowing " << node->state << std::endl;

	nodes.insert(Nodes::value_type(cost, node));
}

void Planner9::success(const Plan& plan) {
	plans.push_back(plan);
}
