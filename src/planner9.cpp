#include "planner9.hpp"
#include "domain.hpp"
#include "problem.hpp"
#include "logic.hpp"
#include "relations.hpp"
#include <iostream>
#include <set>

// debug housekeeping
#ifdef NDEBUG
#undef DEBUG
#endif


// nodes in our search tree
struct TreeNode {
	TreeNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state);
	friend std::ostream& operator<<(std::ostream& os, const TreeNode& node);
	
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

std::ostream& operator<<(std::ostream& os, const TreeNode& node) {
	os << "node " << (&node) << " costs " << node.cost << std::endl;
	os << "after " << node.plan << std::endl;
	os << "do " << node.network << std::endl;
	os << "such that " << node.preconditions << std::endl;
	os << "knowing " << node.state << std::endl;
	return os;
}


Planner9::Planner9(const Problem& problem, std::ostream* debugStream):
	debugStream(debugStream),
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

	if (debugStream)
		*debugStream << "- " << *node << std::endl;

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
		assert(preconditions.empty());
		// TODO: implement
		if (debugStream) *debugStream << preconditions << std::endl;
		success(plan);
	}

	// HTN: nondeterministically choose any t ∈ T0
	for (Tasks::const_iterator taskIt = t0.begin(); taskIt != t0.end(); ++taskIt)
	{
		const Task* t = *taskIt;
		const Head* head = t->head;

		const Action* action = dynamic_cast<const Action*>(head);
		if(action != 0) {
			if (debugStream)
				*debugStream << "action" << std::endl;
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

			if (debugStream) *debugStream << "raw pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;
			Scope::OptionalIndices simplificationResult = newPreconditions.simplify(state, problem.scope.getSize(), newAllocatedVariablesCount);
			if (simplificationResult) {
				if (debugStream) *debugStream << "simp. pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;

				Action::Effects effects(action->getEffects());
				effects.substitute(subst);

				Plan newPlan(plan);

				Scope::Indices simplificationSubst(simplificationResult.get());
				newAllocatedVariablesCount = simplificationSubst.defrag(problem.scope.getSize());
				newPreconditions.substitute(simplificationSubst);
				newPlan.substitute(simplificationSubst);
				newNetwork.substitute(simplificationSubst);
				effects.substitute(simplificationSubst);

				// discover which variables must be grounded

				// collect all variables and relation affected by the effects
				Scope::IndexSet affectedVariables;
				{
					typedef std::set<const Relation*> AffectedRelations;
					AffectedRelations affectedRelations;

					// first iterate on all effects
					for(Action::Effects::const_iterator it = effects.begin(); it != effects.end(); ++it) {
						const Literal& literal = *it;
						affectedRelations.insert(literal.atom.relation);
						for (Scope::Indices::const_iterator jt = literal.atom.params.begin(); jt != literal.atom.params.end(); ++jt) {
							const Scope::Index index = *jt;
							if(index >= problem.scope.getSize())
								affectedVariables.insert(index);
						}
					}

					// then look into preconditions for all indirectly affected variables
					for(CNF::const_iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
						for(Clause::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
							const Atom& atom = jt->atom;
							if (affectedRelations.find(atom.relation) != affectedRelations.end()) {
								// the relation of this atom is affected, all its variables must be grounded
								for (Scope::Indices::const_iterator kt = atom.params.begin(); kt != atom.params.end(); ++kt) {
									const Scope::Index index = *kt;
									if(index >= problem.scope.getSize())
										affectedVariables.insert(index);
								}
							}
						}
					}
				}

				// create a list of affected variables along with their ranges, set their range to maximum
				typedef Relation::VariableRange VariableRange;
				typedef Relation::VariablesRanges VariablesRanges;
				VariablesRanges variablesRanges;
				for (Scope::IndexSet::const_iterator it = affectedVariables.begin(); it != affectedVariables.end(); ++it) {
					const Scope::Index index = *it;
					variablesRanges[index] = VariableRange(problem.scope.getSize(), true);
				}

				// filter out variables ranges using state
				for(CNF::iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
					VariablesRanges clauseRanges;
					for(CNF::Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
						const Literal& literal = *jt;
						const Atom& atom = literal.atom;
						VariablesRanges atomRanges(atom.relation->getRange(atom, state, problem.scope.getSize()));
						// extend range of variables if it is to be grounded
						for (VariablesRanges::iterator kt = atomRanges.begin(); kt != atomRanges.end(); ++kt) {
							const Scope::Index& index = kt->first;
							// TODO: optimize this with an index check
							if (affectedVariables.find(index) != affectedVariables.end()) {
								VariableRange& paramRange = kt->second;
								if(literal.negated)
									~paramRange;
								VariablesRanges::iterator clauseRangesIt = clauseRanges.find(index);
								if (clauseRangesIt != clauseRanges.end()) {
									VariableRange& range = clauseRangesIt->second;
									range |= paramRange;
								} else {
									clauseRanges[index] = paramRange;
								}
							}
						}
					}

					// filtered global ranges with ranges of this disjunction
					for (VariablesRanges::const_iterator jt = clauseRanges.begin(); jt != clauseRanges.end(); ++jt) {
						VariablesRanges::iterator variablesRangesIt = variablesRanges.find(jt->first);
						if (variablesRangesIt != variablesRanges.end()) {
							VariableRange& range = variablesRangesIt->second;
							range &= jt->second;
						} else {
							assert(false);
							variablesRanges[jt->first] = jt->second;
						}
					}
				}

				if (debugStream) {
					*debugStream << "constants " << problem.scope << std::endl;
					*debugStream << "assigning";
					for(VariablesRanges::const_iterator it = variablesRanges.begin(); it != variablesRanges.end(); ++it) {
						*debugStream << " var" << it->first - problem.scope.getSize() << " " << it->second ;
					}
					*debugStream << std::endl;
				}

				// if any of the variable has no domain, return
				for(VariablesRanges::const_iterator it = variablesRanges.begin(); it != variablesRanges.end(); ++it) {
					if (it->second.isEmpty()) {
						return;
					}
				}
				
				// DPLL (http://en.wikipedia.org/wiki/DPLL_algorithm)
				typedef std::pair<Scope::Indices, CNF> Grounding;
				typedef std::vector<Grounding> Groundings;
				Groundings groundings(1, std::make_pair(Scope::Indices::identity(newAllocatedVariablesCount), newPreconditions));
				for(VariablesRanges::const_iterator it = variablesRanges.begin(); it != variablesRanges.end() && !groundings.empty(); ++it) {
					const VariablesRanges::value_type& variable = *it;
					const Scope::Index index = variable.first;
					const VariableRange& range = variable.second;
					Groundings newGroundings;
					for (Groundings::const_iterator kt = groundings.begin(); kt != groundings.end(); ++kt) {
						const Scope::Indices& subst(kt->first);
						const CNF& pre(kt->second);
						if(subst[index] == index) {
							for (size_t jt = 0; jt < range.size(); ++jt) {
								const bool isPossible = range[jt];
								if (isPossible) {
									Scope::Indices newSubst(subst);
									newSubst[index] = jt;
									CNF newPre(pre);
									newPre.substitute(newSubst);
									Scope::OptionalIndices simplificationResult = newPre.simplify(state, problem.scope.getSize(), newAllocatedVariablesCount);
									if (simplificationResult) {
										newSubst.substitute(simplificationResult.get());
										newGroundings.push_back(std::make_pair(newSubst, newPre));
									}
								}
							}
						} else {
							newGroundings.push_back(*kt);
						}
					}
					std::swap(groundings, newGroundings);
				}
				
				// Create new nodes with valid groundings
				for (Groundings::iterator it = groundings.begin(); it != groundings.end(); ++it) {
					Scope::Indices& subst(it->first);
					CNF& remainingPreconditions(it->second);
					size_t assignedAllocatedVariablesCount = subst.defrag(problem.scope.getSize());
					remainingPreconditions.substitute(subst);
					
					// create new task network
					TaskNetwork assignedNetwork(newNetwork.clone());
					assignedNetwork.substitute(subst);
					
					// HTN: append a to P
					Plan assignedPlan(newPlan);
					assignedPlan.push_back(*t);
					assignedPlan.substitute(subst);

					// apply effects
					const State newState = effects.apply(state, subst);

					// HTN: T0 ← {t ∈ T : no task in T is constrained to precede t}
					addNode(assignedPlan, assignedNetwork, assignedAllocatedVariablesCount, cost, remainingPreconditions, newState);
				}
			} else {
				if (debugStream) *debugStream << "simp. pre failed" << std::endl;
			}
		}

		// if t is a method then decompose
		const Method* method = dynamic_cast<const Method*>(head);
		if (method != 0) {
			if (debugStream) *debugStream << "method" << std::endl;
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

				if (debugStream) *debugStream << "* alternative " << alternative.name << std::endl;
				CNF newPreconditions(alternative.precondition);
				newPreconditions.substitute(subst);
				newPreconditions += preconditions;
				if (debugStream) *debugStream << "raw pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;
				Scope::OptionalIndices simplificationResult = newPreconditions.simplify(state, problem.scope.getSize(), newAllocatedVariablesCount);
				if (simplificationResult) {
					if (debugStream) *debugStream << "simp. pre:  " << Scope::setScope(problem.scope) << newPreconditions << std::endl;
	
					Plan newPlan(plan);
	
					// HTN: modify T by removing t, adding sub(m), constraining each task
					// HTN: in sub(m) to precede the tasks that t preceded, and applying θ
					TaskNetwork decomposition(alternative.tasks.clone());
					decomposition.substitute(subst);
					TaskNetwork newNetwork = network.replace(taskIt, decomposition);
					
					Scope::Indices simplificationSubst(simplificationResult.get());
					newAllocatedVariablesCount = simplificationSubst.defrag(problem.scope.getSize());
					newPreconditions.substitute(simplificationSubst);
					newPlan.substitute(simplificationSubst);
					newNetwork.substitute(simplificationSubst);
					
					Cost newCost = cost + alternative.cost;
	
					// HTN: if sub(m) = ∅ then
					// HTN: T0 ← {t ∈ sub(m) : no task in T is constrained to precede t}
					// HTN: else T0 ← {t ∈ T : no task in T is constrained to precede t}
					addNode(newPlan, newNetwork, newAllocatedVariablesCount, newCost, newPreconditions, state);
				} else {
					if (debugStream)
						*debugStream << "simp. pre failed" << std::endl;
				}
			}
		}
	}
}

void Planner9::addNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state) {
	TreeNode* node = new TreeNode(plan, network, allocatedVariablesCount, cost, preconditions, state);

	cost += network.getSize();
	
	if (debugStream)
		*debugStream << "+ " << *node << std::endl;

	nodes.insert(Nodes::value_type(cost, node));
}

void Planner9::success(const Plan& plan) {
	plans.push_back(plan);
}
