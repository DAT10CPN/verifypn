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
                                bool preserveDeadlocks) {

        // Do nothing
        _applications++;

        return false;
            bool continueReductions = false;

            for (uint32_t pid = 0; pid < parent->numberOfPlaces(); pid++) {
                if (hasTimedout())
                    return false;
                if (parent->originalNumberOfTransitions() * 2 < numberOfUnskippedTransitions())
                    return false;


                const Place &place = parent->_places[pid];

                // S5.1, S6.1
                if (place.skip || place.inhib || placeInQuery[pid] > 0 || place.producers.empty() ||
                    place.consumers.empty())
                    continue;

                // Performance consideration
                if (place.producers.size() > explosion_limiter){
                    continueReductions = true;
                    continue;
                }

                // Check that prod and cons are disjoint
                // S3
                const auto presize = place.producers.size();
                const auto postsize = place.consumers.size();
                bool ok = true;
                uint32_t i = 0, j = 0;
                while (i < presize && j < postsize) {
                    if (place.producers[i] < place.consumers[j])
                        i++;
                    else if (place.consumers[j] < place.producers[i])
                        j++;
                    else {
                        ok = false;
                        break;
                    }
                }

                if (!ok) continue;

                // S2
                std::vector<bool> todo (place.consumers.size(), true);
                bool todoAllGood = true;
                // S10-11; Do we need to check?
                std::vector<bool> kIsAlwaysOne (place.consumers.size(), true);

                for (const auto& prod : place.producers){
                    Transition& producer = getTransition(prod);
                    // S4, S6.2
                    if(producer.inhib || producer.post.size() != 1){
                        ok = false;
                        break;
                    }

                    uint32_t kw = getOutArc(producer, pid)->weight;
                    for (uint32_t n = 0; n < place.consumers.size(); n++) {
                        uint32_t w = getInArc(pid, getTransition(place.consumers[n]))->weight;
                        // S1, S9
                        if (parent->initialMarking[pid] >= w || kw % w != 0) {
                            todo[n] = false;
                            todoAllGood = false;
                            continue;
                        } else if (kw != w) {
                            kIsAlwaysOne[n] = false;
                        }
                    }

                    // Check if we have any qualifying consumers left
                    if (!todoAllGood && std::lower_bound(todo.begin(), todo.end(), true) == todo.end()){
                        ok = false;
                        break;
                    }

                    for (const auto& prearc : producer.pre){
                        const auto& preplace = parent->_places[prearc.place];
                        // S6.3, S5.2
                        if (preplace.inhib || placeInQuery[prearc.place] > 0){
                            ok = false;
                            break;
                        } else if (!remove_loops) {
                            // If we can remove loops, that means we are not doing deadlock, so we can do free agglomeration which avoids this condition

                            // S7
                            for(const auto& precons : preplace.consumers){
                                // S7; Transitions in place.producers are exempt from this check
                                if (std::lower_bound(place.producers.begin(), place.producers.end(), precons) != place.producers.end())
                                    continue;

                                Transition& preconsumer = getTransition(precons);
                                // S7; Transitions outside place.producers are not allowed the ability to disable an enabled transition in place.producers
                                if (getInArc(prearc.place, preconsumer)->weight > getOutArc(preconsumer, prearc.place)->weight){
                                    ok = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (!ok) break;
                }

                if (!ok) continue;
                std::vector<uint32_t> originalConsumers = place.consumers;
                std::vector<uint32_t> originalProducers = place.producers;
                for (uint32_t n = 0; n < originalConsumers.size(); n++)
                {
                    if (hasTimedout())
                        return false;
                    if (!todo[n])
                        continue;

                    ok = true;
                    Transition &consumer = getTransition(originalConsumers[n]);
                    // (S8 || S11)
                    if ((!remove_loops || !kIsAlwaysOne[n]) && consumer.pre.size() != 1) {
                        continue;
                    }
                    // S10
                    if (consumer.inhib) {
                        // This is disallowed for performance, so we don't need another full pass of place.producers to ensure we don't mix consumers with inhibitors.
                        // If support for inhibitors and consumers between the same (Transition->Place) is ever added, this rule will work with "&& k[n] > 1".
                        ok = false;
                    }

                    if (!ok) continue;
                    // Update
                    for (const auto& prod : originalProducers){
                        Transition &producer = getTransition(prod);
                        // w is never used unless (kIsAlwaysOne[n]) = true, so no need to initialize it to an actual value.
                        uint32_t k = 1, w = 1;
                        if (!kIsAlwaysOne[n]){
                            w = getInArc(pid, consumer)->weight;
                            k = getOutArc(producer, pid)->weight / w;
                        }

                        // One for each number of firings of consumer possible after one firing of producer
                        for (uint32_t k_i = 1; k_i <= k; k_i++){
                            // Create new transition with effect of firing the producer, and then the consumer k_i times
                            auto id = parent->_transitions.size();
                            if (!_skippedTransitions.empty())
                            {
                                id = _skippedTransitions.back();
                                _skippedTransitions.pop_back();
                            }
                            else
                            {
                                parent->_transitions.emplace_back();
                                parent->_transitionnames[newTransName()] = id;
                                parent->_transitionlocations.emplace_back(std::tuple<double, double>(0.0, 0.0));
                            }

                            // Re-fetch the transition pointers as it might be invalidated, I think that's the issue?
                            Transition &producerPrime = getTransition(prod);
                            Transition &consumerPrime = getTransition(originalConsumers[n]);
                            Transition& newtran = parent->_transitions[id];
                            newtran.skip = false;
                            newtran.inhib = false;

                            // Arcs from consumer
                            for (const auto& arc : consumerPrime.post) {
                                Arc newarc = arc;
                                newarc.weight = newarc.weight * k_i;
                                newtran.addPostArc(newarc);
                            }
                            for (const auto& arc : consumerPrime.pre){
                                if (arc.place != pid){
                                    Arc newarc = arc;
                                    newarc.weight = newarc.weight * k_i;
                                    newtran.addPreArc(arc);
                                }
                            }

                            for (const auto& arc : producerPrime.pre){
                                newtran.addPreArc(arc);
                            }

                            if (k_i != k){
                                Arc newarc = producerPrime.post[0];
                                newarc.weight = (k-k_i)*w;
                                newtran.addPostArc(newarc);
                            }

                            for(const auto& arc : newtran.pre)
                                parent->_places[arc.place].addConsumer(id);
                            for(const auto& arc : newtran.post)
                                parent->_places[arc.place].addProducer(id);
                        }
                    }
                    skipTransition(originalConsumers[n]);
                    continueReductions = true;
                    _ruleS++;
                }

                if (place.consumers.empty()) {
                    if (remove_consumers){
                        // The producers of place will become purely consuming transitions when it is gone, which can sometimes be removed
                        auto transitions = place.producers;
                        for (auto tran_id : transitions)
                            skipTransition(tran_id);
                    }
                    skipPlace(pid);
                }

                consistent();
            }

            return continueReductions;
        }
    }
}