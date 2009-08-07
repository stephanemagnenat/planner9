#include "planner9.hpp"
#include "problem.hpp"
#include "relations.hpp"
#include <iostream>
#include <set>

// debug housekeeping
#ifdef NDEBUG
#undef DEBUG
#endif


const Planner9::Cost Planner9::InfiniteCost = std::numeric_limits<int>::max();

Planner9::SearchNode::SearchNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state):
	plan(plan),
	network(network),
	allocatedVariablesCount(allocatedVariablesCount),
	cost(cost),
	preconditions(preconditions),
	state(state) {
}

std::ostream& operator<<(std::ostream& os, const Planner9::SearchNode& node) {
	os << "node " << (&node) << " costs " << node.cost << std::endl;
	os << "after " << node.plan << std::endl;
	os << "do " << node.network << std::endl;
	os << "such that " << node.preconditions << std::endl;
	os << "knowing " << node.state << std::endl;
	return os;
}

Planner9::Cost Planner9::SearchNode::getTotalCost() const {
	return cost + network.first.size() + network.predecessors.size();
	//return cost + plan.size() + (network.first.size() + network.predecessors.size());
}


Planner9::Planner9(const Scope& problemScope, std::ostream* debugStream):
	problemScope(problemScope),
	debugStream(debugStream) {
	if (debugStream) *debugStream << Scope::setScope(this->problemScope); 
}

void Planner9::visitNode(const SearchNode* n) {
	visitNode(n->plan, n->network, n->allocatedVariablesCount, n->cost, n->preconditions, n->state);
}

void Planner9::visitNode(const Plan& plan, const TaskNetwork& network, size_t allocatedVariablesCount, Cost cost, const CNF& preconditions, const State& state) {
	// HTN: T0 ← {t ∈ T : no other task in T is constrained to precede t}
	const TaskNetwork::Tasks& t0 = network.first;
	
	// HTN: if T = ∅ then return P
	if (t0.empty()) {
		assert(preconditions.empty());
		// TODO: implement
		if (debugStream) *debugStream << preconditions << std::endl;
		success(plan);
	}

	// HTN: nondeterministically choose any t ∈ T0
	for (size_t ti = 0; ti < t0.size(); ++ti)
	{
		const Task& t(t0[ti]->task);
		const Head* head(t.head);

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

			TaskNetwork newNetwork(network);
			newNetwork.erase(ti);

			Substitution subst = t.getSubstitution(action->getScope().getSize(), allocatedVariablesCount);
			size_t newAllocatedVariablesCount = allocatedVariablesCount + action->getScope().getSize() - head->getParamsCount();

			CNF newPreconditions(action->getPrecondition());
			newPreconditions.substitute(subst);
			newPreconditions += preconditions;

			if (debugStream) *debugStream << "raw pre:  " << Scope::setScope(problemScope) << newPreconditions << std::endl;
			OptionalVariables simplificationResult = newPreconditions.simplify(state, problemScope.getSize(), newAllocatedVariablesCount);
			if (simplificationResult) {
				if (debugStream) *debugStream << "simp. pre:  " << Scope::setScope(problemScope) << newPreconditions << std::endl;

				Action::Effects effects(action->getEffects());
				effects.substitute(subst);

				Plan newPlan(plan);

				Substitution simplificationSubst(simplificationResult.get());
				newAllocatedVariablesCount = simplificationSubst.defrag(problemScope.getSize());
				newPreconditions.substitute(simplificationSubst);
				newPlan.substitute(simplificationSubst);
				newNetwork.substitute(simplificationSubst);
				effects.substitute(simplificationSubst);

				// discover which variables must be grounded

				// collect all variables and relation affected by the effects
				VariableSet affectedVariables;
				{
					typedef std::set<const Relation*> AffectedRelations;
					AffectedRelations affectedRelations;

					// first iterate on all effects
					for(Action::Effects::const_iterator it = effects.begin(); it != effects.end(); ++it) {
						const Literal& literal = *it;
						affectedRelations.insert(literal.atom.relation);
						for (Variables::const_iterator jt = literal.atom.params.begin(); jt != literal.atom.params.end(); ++jt) {
							const Variable& variable = *jt;
							if(variable.index >= problemScope.getSize())
								affectedVariables.insert(variable);
						}
					}

					// then look into preconditions for all indirectly affected variables
					for(CNF::const_iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
						for(Clause::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
							const Atom& atom = jt->atom;
							if (affectedRelations.find(atom.relation) != affectedRelations.end()) {
								// the relation of this atom is affected, all its variables must be grounded
								for (Variables::const_iterator kt = atom.params.begin(); kt != atom.params.end(); ++kt) {
									const Variable& variable = *kt;
									if(variable.index >= problemScope.getSize())
										affectedVariables.insert(variable);
								}
							}
						}
					}
				}

				// create a list of affected variables along with their ranges, set their range to maximum
				VariablesRanges variablesRanges;
				for (VariableSet::const_iterator it = affectedVariables.begin(); it != affectedVariables.end(); ++it) {
					const Variable& variable = *it;
					variablesRanges[variable] = VariableRange(problemScope.getSize(), true);
				}

				// filter out variables ranges using state
				for(CNF::iterator it = newPreconditions.begin(); it != newPreconditions.end(); ++it) {
					VariablesRanges clauseRanges;
					for(CNF::Disjunction::iterator jt = it->begin(); jt != it->end(); ++jt) {
						const Literal& literal = *jt;
						const Atom& atom = literal.atom;
						VariablesRanges atomRanges(atom.predicate->getRange(state, problemScope.getSize()));
						// extend range of variables if it is to be grounded
						for (VariablesRanges::iterator kt = atomRanges.begin(); kt != atomRanges.end(); ++kt) {
							const Variable& variable = kt->first;
							// TODO: optimize this with an index check
							if (affectedVariables.find(variable) != affectedVariables.end()) {
								VariableRange& paramRange = kt->second;
								if(literal.negated)
									~paramRange;
								VariablesRanges::iterator clauseRangesIt = clauseRanges.find(variable);
								if (clauseRangesIt != clauseRanges.end()) {
									VariableRange& range = clauseRangesIt->second;
									range |= paramRange;
								} else {
									clauseRanges[variable] = paramRange;
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
					*debugStream << "constants " << problemScope << std::endl;
					*debugStream << "assigning";
					for(VariablesRanges::const_iterator it = variablesRanges.begin(); it != variablesRanges.end(); ++it) {
						*debugStream << " var" << it->first.index - problemScope.getSize() << " " << it->second ;
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
				typedef std::pair<Substitution, CNF> Grounding;
				typedef std::vector<Grounding> Groundings;
				Groundings groundings(1, std::make_pair(Substitution::identity(newAllocatedVariablesCount), newPreconditions));
				for(VariablesRanges::const_iterator it = variablesRanges.begin(); it != variablesRanges.end() && !groundings.empty(); ++it) {
					const VariablesRanges::value_type& pair = *it;
					const Variable& variable = pair.first;
					const VariableRange& range = pair.second;
					Groundings newGroundings;
					for (Groundings::const_iterator kt = groundings.begin(); kt != groundings.end(); ++kt) {
						const Substitution& subst(kt->first);
						const CNF& pre(kt->second);
						if(subst[variable.index] == variable) {
							for (size_t jt = 0; jt < range.size(); ++jt) {
								const bool isPossible = range[jt];
								if (isPossible) {
									Substitution newSubst(subst);
									newSubst[variable.index] = Variable(jt);
									CNF newPre(pre);
									newPre.substitute(newSubst);
									OptionalVariables simplificationResult = newPre.simplify(state, problemScope.getSize(), newAllocatedVariablesCount);
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
					Substitution& subst(it->first);
					CNF& remainingPreconditions(it->second);
					size_t assignedAllocatedVariablesCount = subst.defrag(problemScope.getSize());
					remainingPreconditions.substitute(subst);

					// create new task network
					TaskNetwork assignedNetwork(newNetwork);
					assignedNetwork.substitute(subst);

					// HTN: append a to P
					Plan assignedPlan(newPlan);
					assignedPlan.push_back(t);
					assignedPlan.substitute(subst);

					// apply effects
					const State newState = effects.apply(state, subst);

					// HTN: T0 ← {t ∈ T : no task in T is constrained to precede t}
					pushNode(assignedPlan, assignedNetwork, assignedAllocatedVariablesCount, cost, remainingPreconditions, newState);
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

				Substitution subst = t.getSubstitution(alternative.scope.getSize(), allocatedVariablesCount);
				size_t newAllocatedVariablesCount = allocatedVariablesCount + alternative.scope.getSize() - head->getParamsCount();

				if (debugStream) *debugStream << "* alternative " << alternative.name << std::endl;
				CNF newPreconditions(alternative.precondition);
				newPreconditions.substitute(subst);
				newPreconditions += preconditions;
				if (debugStream) *debugStream << "raw pre:  " << Scope::setScope(problemScope) << newPreconditions << std::endl;
				OptionalVariables simplificationResult = newPreconditions.simplify(state, problemScope.getSize(), newAllocatedVariablesCount);
				if (simplificationResult) {
					if (debugStream) *debugStream << "simp. pre:  " << Scope::setScope(problemScope) << newPreconditions << std::endl;

					Plan newPlan(plan);

					// HTN: modify T by removing t, adding sub(m), constraining each task
					// HTN: in sub(m) to precede the tasks that t preceded, and applying θ
					TaskNetwork decomposition(alternative.tasks);
					decomposition.substitute(subst);
					TaskNetwork newNetwork(network);
					newNetwork.replace(ti, decomposition);
					
					Substitution simplificationSubst(simplificationResult.get());
					newAllocatedVariablesCount = simplificationSubst.defrag(problemScope.getSize());
					newPreconditions.substitute(simplificationSubst);
					newPlan.substitute(simplificationSubst);
					newNetwork.substitute(simplificationSubst);

					Cost newCost = cost + alternative.cost;

					// HTN: if sub(m) = ∅ then
					// HTN: T0 ← {t ∈ sub(m) : no task in T is constrained to precede t}
					// HTN: else T0 ← {t ∈ T : no task in T is constrained to precede t}
					pushNode(newPlan, newNetwork, newAllocatedVariablesCount, newCost, newPreconditions, state);
				} else {
					if (debugStream)
						*debugStream << "simp. pre failed" << std::endl;
				}
			}
		}
	}
}

void Planner9::pushNode(const Plan& plan, const TaskNetwork& network, size_t freeVariablesCount, Cost cost, const CNF& preconditions, const State& state) {
	pushNode(new SearchNode(plan, network, freeVariablesCount, cost, preconditions, state));
}


SimplePlanner9::SimplePlanner9(const Scope& problemScope, std::ostream* debugStream):
	Planner9(problemScope, debugStream),
	iterationCount(0) {
}

SimplePlanner9::SimplePlanner9(const Problem& problem, std::ostream* debugStream):
	Planner9(problem.scope, debugStream),
	iterationCount(0) {
	
	// HTN: P = the empty plan
	Planner9::pushNode(Plan(), problem.network, problemScope.getSize(), 0, CNF(), problem.state);
	
}

SimplePlanner9::~SimplePlanner9() {
	for (SearchNodes::iterator it = nodes.begin(); it != nodes.end(); ++it)
		delete it->second;
}

// HTN: procedure SHOP2(s, T, D)
boost::optional<Plan> SimplePlanner9::plan() {
	// HTN: loop
	while (plan(1))  {
	}
	
	std::cout << "Terminated after " << iterationCount << " iterations" << std::endl;

	if(plans.empty())
		return false;
	else
		return plans.front();
}

bool SimplePlanner9::plan(size_t steps) {
	size_t iterationMax = iterationCount + steps;
	
	// HTN: loop
	while (plans.empty() && !nodes.empty() && (iterationCount < iterationMax))  {
		SearchNode* node = popNode();
		
		if (debugStream)
			*debugStream << "- " << *node << std::endl;
		
		++iterationCount;
		visitNode(node);
		
		delete node;
	}
	
	if (!plans.empty() || nodes.empty())
		return false;
	else
		return true;
}

Planner9::SearchNode* SimplePlanner9::popNode() {
	SearchNodes::iterator front = nodes.begin();
	SearchNode* node = front->second;
	nodes.erase(front);
	return node;
}

void SimplePlanner9::pushNode(SearchNode* node) {
	if (debugStream)
		*debugStream << "+ " << *node << std::endl;

	nodes.insert(SearchNodes::value_type(node->getTotalCost(), node));
}

void SimplePlanner9::success(const Plan& plan) {
	plans.push_back(plan);
}
