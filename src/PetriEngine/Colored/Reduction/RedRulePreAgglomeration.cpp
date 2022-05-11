/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include <PetriEngine/Colored/VarReplaceVisitor.h>
#include "PetriEngine/Colored/Reduction/RedRulePreAgglomeration.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/Colored/ArcVarMultisetVisitor.h"
#include "PetriEngine/Colored/VariableVisitor.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreAgglomeration::isApplicable(QueryType queryType, bool preserveLoops, bool preserveStutter) const {
        return queryType != CTL && !preserveStutter;
    }

    bool RedRulePreAgglomeration::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                        QueryType queryType, bool preserveLoops, bool preserveStutter) {
        bool continueReductions = false;
        bool changed = true;
        bool atomic_viable = (queryType == Reach) && !preserveLoops;

        // Apply repeatedly
        while (changed) {
            changed = false;

            for (uint32_t pid = 0; pid < red.placeCount(); pid++) {
                if (red.hasTimedOut())
                    return false;
                // Limit explosion
                if (red.origTransitionCount() * 2 < red.unskippedTransitionsCount())
                    return false;

                const Place &place = red.places()[pid];

                // Limit large applications
                if (place._pre.size() > explosion_limiter){
                    continue;
                }

                // T/S8--1, T/S1, T/S3
                if (place.skipped || place.inhibitor || inQuery.isPlaceUsed(pid) || !place.marking.empty() || place._pre.empty() ||
                    place._post.empty())
                    continue;


                const auto presize = place._pre.size();
                const auto postsize = place._post.size();
                bool ok = true;
                uint32_t i = 0, j = 0;

                // Check that producers and consumers are disjoint, and not in a fireability query
                // T/S4, T/S2
                while (i < presize && j < postsize) {
                    if (place._pre[i] < place._post[j]) {
                        if (inQuery.isTransitionUsed(place._pre[i])) {
                            ok = false;
                            break;
                        }
                        i++;
                    }
                    else if (place._post[j] < place._pre[i]) {
                        if (inQuery.isTransitionUsed(place._post[j])) {
                            ok = false;
                            break;
                        }
                        j++;
                    }
                    else {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;

                for ( ; i < presize; i++) {
                    if (inQuery.isTransitionUsed(place._pre[i])) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;

                for ( ; j < postsize; j++) {
                    if (inQuery.isTransitionUsed(place._post[j])) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) continue;

                std::vector<bool> todo (postsize, true);
                bool todoAllGood = true;
                // S11, S12
                std::vector<bool> kIsAlwaysOne (postsize, true);

                for (const auto& prod : place._pre){
                    const Transition& producer = red.transitions()[prod];
                    // X8.1, X6
                    if(producer.inhibited || producer.output_arcs.size() != 1){
                        ok = false;
                        break;
                    }

                    const CArcIter& prodArc = red.getOutArc(producer, pid);
                    uint32_t kw;

                    // T9, S6
                    if(prodArc->expr->is_single_color()){
                        kw = prodArc->expr->weight();
                    } else {
                        ok = false;
                        break;
                    }

                    for (uint32_t n = 0; n < place._post.size(); n++) {
                        const PetriEngine::Colored::Transition& consumer = red.transitions()[place._post[n]];
                        const CArcIter& consArc = red.getInArc(pid, consumer);
                        uint32_t w = consArc->expr->weight();
                        // (T9, S6), S10, T10, T12
                        if (atomic_viable){
                            if (!consArc->expr->is_single_color() || kw % w != 0) {
                                todo[n] = false;
                                todoAllGood = false;
                            } else if (kw != w) {
                                kIsAlwaysOne[n] = false;
                            }
                        } else if (!consArc->expr->is_single_color() || kw != w || consumer.input_arcs.size() != 1) {
                            ok = false;
                            break;
                        }
                    }

                    // Check if we have any qualifying consumers left
                    if (!ok || (!todoAllGood && std::lower_bound(todo.begin(), todo.end(), true) == todo.end())){
                        ok = false;
                        break;
                    }

                    for (const auto& prearc : producer.input_arcs){
                        const Place& preplace = red.places()[prearc.place];
                        // T/S8--3, T/S7--2
                        if (preplace.inhibitor || inQuery.isPlaceUsed(prearc.place)){
                            ok = false;
                            break;
                        } else if (!atomic_viable) {
                            // For reachability, we can do free agglomeration which avoids this condition
                            // T5
                            for(uint32_t alternative : preplace._post){
                                // T5; Transitions in place.pre are exempt from this check
                                if (std::lower_bound(place._pre.begin(), place._pre.end(), alternative) != place._pre.end())
                                    continue;

                                const Transition& alternativeConsumer = red.transitions()[alternative];
                                // T5; Transitions outside place.pre are not allowed to alter the contents of preplace
                                if (red.getInArc(prearc.place, alternativeConsumer)->expr == red.getOutArc(alternativeConsumer, prearc.place)->expr){
                                    ok = false;
                                    break;
                                }
                            }
                        }
                        if (!ok) break;
                    }

                    if (!ok) break;
                }

                if (!ok) continue;

                std::vector<uint32_t> originalConsumers = place._post;
                std::vector<uint32_t> originalProducers = place._pre;
                // a pair containing prodHangingGuardVarRisk, prodHangingArcVar
                std::pair<bool, bool> prodHangingVars = _prodHangingGuardVar(red, pid, originalProducers);

                for (uint32_t n = 0; n < originalConsumers.size(); n++)
                {
                    if (red.hasTimedOut())
                        return false;
                    if (!todo[n])
                        continue;
                    ok = true;

                    const Transition &consumer = red.transitions()[originalConsumers[n]];

                    if (atomic_viable){
                        // S12
                        if (!kIsAlwaysOne[n] && consumer.input_arcs.size() != 1) {
                            continue;
                        }
                        // S11
                        if (!kIsAlwaysOne[n]) {
                            for (const auto& conspost : consumer.output_arcs) {
                                if (red.places()[conspost.place].inhibitor) {
                                    ok = false;
                                    break;
                                }
                            }
                        }
                        if (!ok) continue;
                    }

                    const auto& consArc = red.getInArc(pid, consumer);
                    uint32_t w = consArc->expr->weight();

                    // Identify the variables of the consumer
                    bool consHangingGuardVarRisk = false;
                    std::set<const Variable*> consVars;
                    std::set<const Variable*> consArcVars;
                    std::set<const Variable*> consGuardVars;
                    if (prodHangingVars.second){
                        // The hanging variable checks need the arc variables and guard variables in separate sets
                        Colored::VariableVisitor::get_variables(*consArc->expr, consArcVars);
                        if(consumer.guard)
                            Colored::VariableVisitor::get_variables(*consumer.guard, consGuardVars);

                        for (auto& var : consArcVars){
                            if (prodHangingVars.second && consGuardVars.find(var) != consGuardVars.end()){
                                // prodHangingArcVar && consHangingGuardVarRisk is NG
                                consHangingGuardVarRisk = true;
                                break;
                            }
                        }
                    } else {
                        if(consumer.guard)
                            Colored::VariableVisitor::get_variables(*consumer.guard, consVars);
                    }

                    for (auto& arc : consumer.input_arcs){
                        if (!prodHangingVars.first || arc.place != consArc->place){
                            Colored::VariableVisitor::get_variables(*arc.expr, consVars);
                        }
                    }
                    for (auto& arc : consumer.output_arcs){
                        if (!prodHangingVars.first || arc.place != consArc->place){
                            Colored::VariableVisitor::get_variables(*arc.expr, consVars);
                        }
                    }

                    // The hanging guards that could not be caught by the producer's arcs have to be caught by the consumer's arcs now, or the agglomeration cant go on.
                    if (prodHangingVars.first || consHangingGuardVarRisk) {
                        for (auto& var : consArcVars){
                            if (consVars.find(var) == consVars.end()){
                                // If the producer has a hanging guard variable, we cannot allow consArc to also have hanging variables
                                ok = false;
                                break;
                            }
                        }
                    }

                    if (!ok){
                        if (atomic_viable){
                            continue;
                        } else {
                            break;
                        }
                    }

                    // Update
                    for (const auto& prod : originalProducers){
                        const Transition& producer = red.transitions()[prod];
                        const CArcIter proArc = red.getOutArc(producer, pid);
                        std::set<const Variable*> pairVars;
                        std::unordered_map<uint32_t, std::vector<const Colored::Variable*>> tuples;
                        // Valid because there is only allowed to be at most 1 variable on each.
                        Colored::VariableVisitor::get_variables(*proArc->expr, pairVars, tuples);
                        Colored::VariableVisitor::get_variables(*consArc->expr, pairVars, tuples);

                        std::vector<Variable*> centerVariables;
                        std::unordered_map<std::string, const Variable*> varReplacementMap;
                        for (auto& tuple : tuples){
                            for (uint32_t tupleIndex = 0; tupleIndex < tuple.second.size(); tupleIndex++){
                                if (centerVariables.size() <= tupleIndex){
                                    auto* newVar = new Variable{*producer.name + *consumer.name + tuple.second[tupleIndex]->name, tuple.second[tupleIndex]->colorType};
                                    red.addVariable(newVar);
                                    centerVariables.emplace_back(newVar);
                                }
                                varReplacementMap[tuple.second[tupleIndex]->name] = centerVariables[tupleIndex];
                            }
                        }
                        pairVars.insert(consVars.begin(), consVars.end());

                        if(producer.guard){
                            Colored::VariableVisitor::get_variables(*producer.guard, pairVars);
                        }
                        for (auto& arc : producer.input_arcs){
                            Colored::VariableVisitor::get_variables(*arc.expr, pairVars);
                        }
                        for (auto& arc : producer.output_arcs){
                            Colored::VariableVisitor::get_variables(*arc.expr, pairVars);
                        }

                        for (auto& var : pairVars){
                            if (varReplacementMap[var->name] == nullptr){
                                auto* newVar = new Variable{*producer.name + *consumer.name + var->name, var->colorType};
                                red.addVariable(newVar);
                                varReplacementMap[var->name] = newVar;
                            }
                        }

                        VarReplaceVisitor varvis = VarReplaceVisitor(varReplacementMap);

                        uint32_t k = 1;
                        if (!kIsAlwaysOne[n]){
                            k = proArc->expr->weight() / w;
                        }
                        GuardExpression_ptr mergedguard = nullptr;
                        if (consumer.guard != nullptr && producer.guard != nullptr){
                            mergedguard = std::make_shared<PetriEngine::Colored::AndExpression>(varvis.makeReplacementGuard(producer.guard), varvis.makeReplacementGuard(consumer.guard));
                        } else if (consumer.guard != nullptr){
                            mergedguard = varvis.makeReplacementGuard(consumer.guard);
                        } else if (producer.guard != nullptr){
                            mergedguard = varvis.makeReplacementGuard(producer.guard);
                        }

                        // One for each number of firings of consumer possible after one firing of producer
                        for (uint32_t k_i = 1; k_i <= k; k_i++){
                            // Create new transition with effect of firing the producer, and then the consumer k_i times
                            auto tid = red.newTransition(mergedguard);

                            // Separate variables for the k_i firing versions.
                            if (k_i > 1){
                                for (auto& kvPair : varReplacementMap) {
                                    auto* newVar = new Variable{kvPair.first + "k" + std::to_string(k_i), kvPair.second->colorType};
                                    red.addVariable(newVar);
                                    kvPair.second = newVar;
                                }
                            }

                            // Re-fetch the transition references as they might be invalidated?
                            const Transition &producerPrime = red.transitions()[prod];
                            const Transition &consumerPrime = red.transitions()[originalConsumers[n]];

                            // Arcs from consumer
                            for (const auto& arc : consumerPrime.output_arcs) {
                                ArcExpression_ptr expr = varvis.makeReplacementArcExpr(arc.expr);
                                if (k_i > 1){
                                    red.addOutputArc(tid, arc.place, std::make_shared<PetriEngine::Colored::ScalarProductExpression>(std::shared_ptr(expr), k_i));
                                } else {
                                    red.addOutputArc(tid, arc.place, expr);
                                }
                            }
                            for (const auto& arc : consumerPrime.input_arcs){
                                if (arc.place != pid){
                                    ArcExpression_ptr expr = varvis.makeReplacementArcExpr(arc.expr);
                                    red.addInputArc(arc.place, tid, expr, arc.inhib_weight);
                                }
                            }

                            for (const auto& arc : producerPrime.input_arcs){
                                ArcExpression_ptr expr = varvis.makeReplacementArcExpr(arc.expr);
                                red.addInputArc(arc.place, tid, expr, arc.inhib_weight);
                            }

                            if (k_i != k){
                                red.addOutputArc(tid, pid, std::make_shared<PetriEngine::Colored::ScalarProductExpression>(varvis.makeReplacementArcExpr(proArc->expr), k-k_i));
                            }
                        }
                    }
                    red.skipTransition(originalConsumers[n]);
                    changed = true;
                    _applications++;
                }

                if (place._post.empty()) {
                    auto transitions = place._pre;
                    for (uint32_t tran_id : transitions)
                        red.skipTransition(tran_id);
                    red.skipPlace(pid);
                }

                red.consistent();
            }

            continueReductions |= changed;
        }

        red.consistent();
        return continueReductions;
    }

    std::pair<bool, bool> RedRulePreAgglomeration::_prodHangingGuardVar(ColoredReducer& red, uint32_t pid, const std::vector<uint32_t>& originalProducers) {
        bool hangingGuardVar_risk = false;
        bool hangingArcVar = false;
        for (const auto& prod : originalProducers){
            const Transition& producer = red.transitions()[prod];
            const CArcIter prodArc = red.getOutArc(producer, pid);
            std::set<const Variable*> prodArcVars;
            std::set<const Variable*> prodGuardVars;

            Colored::VariableVisitor::get_variables(*prodArc->expr, prodArcVars);
            if(producer.guard){
                Colored::VariableVisitor::get_variables(*producer.guard, prodGuardVars);
                for (auto& var : prodArcVars){
                    if (prodGuardVars.find(var) != prodGuardVars.end()){
                        hangingGuardVar_risk = true;
                        break;
                    }
                }
            }

            std::set<const Variable*> prodVars;

            for (auto& arc : producer.input_arcs){
                if (arc.place != prodArc->place){
                    Colored::VariableVisitor::get_variables(*arc.expr, prodVars);
                }
            }
            for (auto& arc : producer.output_arcs){
                if (arc.place != prodArc->place){
                    Colored::VariableVisitor::get_variables(*arc.expr, prodVars);
                }
            }

            for (auto& var : prodArcVars){
                if (prodVars.find(var) == prodVars.end()){
                    // There is indeed a hanging guard variable here;
                    hangingArcVar = true;
                    break;
                }
            }
            if (hangingGuardVar_risk && hangingArcVar) break;
        }
        // hangingGuardVar_risk is only actually a problem if there is a hangingArcVar too.
        return std::pair{(hangingGuardVar_risk && hangingArcVar), hangingArcVar};
    }
}

