/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRulePreAgglomeration.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreAgglomeration::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                        QueryType queryType, bool preserveLoops, bool preserveStutter, uint32_t explosion_limiter) {
        bool continueReductions = false;

        for (uint32_t pid = 0; pid < red.placeCount(); pid++) {
            if (red.hasTimedOut())
                return false;
            if (red.origTransitionCount() * 2 < red.unskippedTransitionsCount())
                return false;


            const Place &place = red.places()[pid];

            // X4, X7.1, X1
            if (place.skipped || place.inhibitor || inQuery[pid] > 0 || !place.marking.empty() || place._pre.empty() ||
                place._post.empty())
                continue;

            // Start small
            if (place._pre.size() > explosion_limiter){
                continueReductions = true;
                continue;
            }

            // Check that producers and consumers are disjoint
            // X3
            const auto presize = place._pre.size();
            const auto postsize = place._post.size();
            bool ok = true;
            uint32_t i = 0, j = 0;
            while (i < presize && j < postsize) {
                if (place._pre[i] < place._post[j])
                    i++;
                else if (place._pre[j] < place._post[i])
                    j++;
                else {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            std::vector<bool> todo (postsize, true);
            bool todoAllGood = true;
            // X14, not supported yet
            std::vector<bool> kIsAlwaysOne (postsize, true);

            // Visitor for translating ArcExpressions into the info we need.
            ArcExpressionVisitor arvis = ArcExpressionVisitor();

            for (const auto& prod : place._pre){
                const Transition& producer = red.transitions()[prod];
                // X8.1, X6
                if(producer.inhibited || producer.input_arcs.size() != 1){
                    ok = false;
                    break;
                }

                // Have arvis visit the arc from producer to place
                arvis.reset();
                red.getOutArc(producer, pid)->expr->visit(arvis);
                uint32_t kw = 1;

                // X5
                if(arvis.ok() && arvis.singleColor()){
                    kw = arvis.colorblindWeight();
                } else {
                    ok = false;
                    break;
                }

                for (uint32_t n = 0; n < place._post.size(); n++) {
                    arvis.reset();
                    red.getInArc(pid, red.transitions()[place._post[n]])->expr->visit(arvis);
                    uint32_t w = arvis.colorblindWeight();
                    // X9, (X5)
                    if (!arvis.ok() || !arvis.singleColor() || kw % w != 0) {
                        todo[n] = false;
                        todoAllGood = false;
                    } else if (kw != w) {
                        kIsAlwaysOne[n] = false;
                    }
                }

                // Check if we have any qualifying consumers left
                if (!todoAllGood && std::lower_bound(todo.begin(), todo.end(), true) == todo.end()){
                    ok = false;
                    break;
                }

                for (const auto& prearc : producer.input_arcs){
                    const Place& preplace = red.places()[prearc.place];
                    // X8.2, X7.2
                    if (preplace.inhibitor || inQuery[prearc.place] > 0){
                        ok = false;
                        break;
                    } else if (queryType != Reach) {
                        // For reachability, we can do free agglomeration which avoids this condition
                        // X10
                        for(uint32_t alternative : preplace._post){
                            // X10; Transitions in place.pre are exempt from this check
                            if (std::lower_bound(place._pre.begin(), place._pre.end(), alternative) != place._pre.end())
                                continue;

                            const Transition& alternativeConsumer = red.transitions()[alternative];
                            // X10; Transitions outside place.pre are not allowed to alter the contents of preplace
                            if (red.getInArc(prearc.place, alternativeConsumer)->expr == red.getOutArc(alternativeConsumer, prearc.place)->expr){
                                ok = false;
                                break;
                            }
                        }
                    }
                }

                if (!ok) break;
            }

            if (!ok) continue;
            std::vector<uint32_t> originalConsumers = place._post;
            std::vector<uint32_t> originalProducers = place._pre;
            for (uint32_t n = 0; n < originalConsumers.size(); n++)
            {
                if (red.hasTimedOut())
                    return false;
                if (!todo[n])
                    continue;

                const Transition &consumer = red.transitions()[originalConsumers[n]];
                // (X10 || X15)
                if ((queryType != Reach || !kIsAlwaysOne[n]) && consumer.input_arcs.size() != 1) {
                    continue;
                }
                // X16?
                if (!kIsAlwaysOne[n]) {
                    for (const auto& conspost : consumer.output_arcs) {
                        if (red.places()[conspost.place].inhibitor)
                            continue;
                    }
                }

                arvis.reset();
                CArcIter consArc = red.getInArc(pid, consumer);
                consArc->expr->visit(arvis);
                uint32_t w = arvis.colorblindWeight();

                // Update
                for (const auto& prod : originalProducers){
                    arvis.reset();
                    const Transition& producer = red.transitions()[prod];
                    CArcIter proArc = red.getOutArc(producer, pid);
                    uint32_t k = 1;
                    if (!kIsAlwaysOne[n]){
                        proArc->expr->visit(arvis);
                        k = arvis.colorblindWeight() / w;
                    }

                    // One for each number of firings of consumer possible after one firing of producer
                    for (uint32_t k_i = 1; k_i <= k; k_i++){
                        // Create new transition with effect of firing the producer, and then the consumer k_i times
                        auto id = red.newTransition(nullptr);

                        // Re-fetch the transition pointers as it might be invalidated, I think that's the issue?
                        const Transition &producerPrime = red.transitions()[prod];
                        const Transition &consumerPrime = red.transitions()[originalConsumers[n]];
                        const Transition& newtran = red.transitions()[id];

                        // Arcs from consumer
                        for (const auto& arc : consumerPrime.output_arcs) {
                            ArcExpression_ptr expr = arc.expr;
                            red.addOutputArc(newtran, red.places()[arc.place], std::make_shared<PetriEngine::Colored::ScalarProductExpression>(std::shared_ptr(expr), k_i));
                        }
                        for (const auto& arc : consumerPrime.input_arcs){
                            if (arc.place != pid){
                                ArcExpression_ptr expr = arc.expr;
                                red.addInputArc(red.places()[arc.place], newtran, expr, arc.inhib_weight);
                            }
                        }

                        for (const auto& arc : producerPrime.input_arcs){
                            ArcExpression_ptr expr = arc.expr;
                            red.addInputArc(red.places()[arc.place], newtran, expr, arc.inhib_weight);
                        }

                        if (k_i != k){
                            red.addOutputArc(newtran, place, std::make_shared<PetriEngine::Colored::ScalarProductExpression>(std::shared_ptr(proArc->expr), k-k_i));
                        }
                    }
                }
                red.skipTransition(originalConsumers[n]);
                continueReductions = true;
                _applications++;
            }

            if (place._post.empty()) {
                if (!preserveStutter){
                    // The producers of place will become purely consuming transitions when it is gone, which can sometimes be removed
                    // The places they consume from aren't allowed to be in the query, but if they were we couldn't reach this point either.
                    auto transitions = place._pre;
                    for (auto tran_id : transitions)
                        red.skipTransition(tran_id);
                }
                red.skipPlace(pid);
            }

            red.consistent();
        }

        return continueReductions;
    }
}