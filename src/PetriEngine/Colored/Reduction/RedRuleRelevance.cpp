/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include <PetriEngine/Colored/ArcVarMultisetVisitor.h>
#include "PetriEngine/Colored/Reduction/RedRuleRelevance.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleRelevance::isApplicable(QueryType queryType, bool preserveLoops, bool preserveStutter) const {
        return false; //!preserveStutter;
    }

    bool RedRuleRelevance::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                        QueryType queryType, bool preserveLoops, bool preserveStutter) {

        bool changed = true;

        // Apply repeatedly
        while (changed) {
            changed = false;

            std::vector<uint32_t> wtrans;
            std::vector<bool> tseen(red.transitions().size(), false);
            std::vector<bool> pseen(red.places().size(), false);

            for (uint32_t pid = 0; pid < red.placeCount(); pid++) {
                if (red.hasTimedOut())
                    return false;

                if (inQuery[pid]){
                    pseen[pid] = true;
                    const Place &place = red.places()[pid];
                    for (auto t : place._post) {
                        if (!tseen[t]) {
                            wtrans.push_back(t);
                            tseen[t] = true;
                        }
                    }
                    for (auto t : place._pre) {
                        if (!tseen[t]) {
                            wtrans.push_back(t);
                            tseen[t] = true;
                        }
                    }
                } else {
                    continue;
                }
            }

            while (!wtrans.empty()) {
                if (red.hasTimedOut()) return false;
                auto t = wtrans.back();
                wtrans.pop_back();
                const Transition &relevantTrans = red.transitions()[t];
                for (const Arc &arc: relevantTrans.input_arcs) {
                    const Place &place = red.places()[arc.place];
                    pseen[arc.place] = true;
                    if (arc.inhib_weight == 0){
                        for (auto prtID : place._pre) {
                            if (!tseen[prtID]) {
                                const PetriEngine::Colored::Transition& potentiallyRelevantTrans = red.transitions()[prtID];
                                auto prtOut = potentiallyRelevantTrans.output_arcs.begin();
                                for (; prtOut != potentiallyRelevantTrans.output_arcs.end(); ++prtOut){
                                    if (prtOut->place >= arc.place) break;
                                }
                                if (prtOut != potentiallyRelevantTrans.output_arcs.end() && prtOut->place == arc.place) {
                                    const auto& prtIn = red.getInArc(arc.place, potentiallyRelevantTrans);
                                    if (auto ms1 = PetriEngine::Colored::extractVarMultiset(*prtIn->expr)){
                                        if (auto ms2 = PetriEngine::Colored::extractVarMultiset(*prtOut->expr)) {
                                            if (ms1 == ms2){
                                                continue;
                                            }
                                        }
                                    }
                                }
                                tseen[prtID] = true;
                                wtrans.push_back(prtID);
                            }
                        }
                    } else {
                        for (auto prtID : place._post) {
                            if (!tseen[prtID]) {
                                // Summary of block: 'pt' is seen unless it:
                                // - Is inhibited by 'place'
                                // - Forms a decreasing loop on 'place' that cannot lower the marking of 'place' below the weight of 'arc'
                                // - Forms a non-decreasing loop on 'place'
                                const Transition &potentiallyRelevantTrans = red.transitions()[prtID];
                                auto prtOut = potentiallyRelevantTrans.output_arcs.begin();
                                for (; prtOut != potentiallyRelevantTrans.output_arcs.end(); ++prtOut)
                                    if (prtOut->place >= arc.place) break;

                                if (prtOut != potentiallyRelevantTrans.output_arcs.end() && prtOut->place == arc.place) {
                                    const auto prtIn = red.getInArc(arc.place, potentiallyRelevantTrans);
                                    // No need for a || it2->place != arc.place condition because we know the loop will always break on it2->place == arc.place
                                    if (prtIn->inhib_weight > 0 || prtOut->expr->weight() >= arc.expr->weight() ||
                                            prtOut->expr->weight() >= prtIn->expr->weight())
                                        continue;
                                }
                                tseen[prtID] = true;
                                wtrans.push_back(prtID);
                            }
                        }
                    }
                }
            }

        }

        return changed;
    }
}

//Reducer::relevant(const uint32_t *placeInQuery, bool remove_consumers) {
//    std::vector<uint32_t> wtrans;
//    std::vector<bool> tseen(parent->numberOfTransitions(), false);
//    for (uint32_t p = 0; p < parent->numberOfPlaces(); ++p) {
//        if (hasTimedout()) return std::nullopt;
//        if (placeInQuery[p] > 0) {
//            const Place &place = parent->_places[p];
//            for (auto t : place.consumers) {
//                if (!tseen[t]) {
//                    wtrans.push_back(t);
//                    tseen[t] = true;
//                }
//            }
//            for (auto t : place.producers) {
//                if (!tseen[t]) {
//                    wtrans.push_back(t);
//                    tseen[t] = true;
//                }
//            }
//        }
//    }
//    std::vector<bool> pseen(parent->numberOfPlaces(), false);
//
//    while (!wtrans.empty()) {
//        if (hasTimedout()) return std::nullopt;
//        auto t = wtrans.back();
//        wtrans.pop_back();
//        const Transition &trans = parent->_transitions[t];
//        for (const Arc &arc : trans.pre) {
//            const Place &place = parent->_places[arc.place];
//            if (arc.inhib) {
//                for (auto pt : place.consumers) {
//                    if (!tseen[pt]) {
//                        // Summary of block: 'pt' is seen unless it:
//                        // - Is inhibited by 'place'
//                        // - Forms a decreasing loop on 'place' that cannot lower the marking of 'place' below the weight of 'arc'
//                        // - Forms a non-decreasing loop on 'place'
//                        Transition &trans = parent->_transitions[pt];
//                        auto it = trans.post.begin();
//                        for (; it != trans.post.end(); ++it)
//                            if (it->place >= arc.place) break;
//
//                        if (it != trans.post.end() && it->place == arc.place) {
//                            auto it2 = trans.pre.begin();
//                            // Find the arc from place to trans we know to exist because that is how we found trans in the first place
//                            for (; it2 != trans.pre.end(); ++it2)
//                                if (it2->place >= arc.place) break;
//                            // No need for a || it2->place != arc.place condition because we know the loop will always break on it2->place == arc.place
//                            if (it2->inhib || it->weight >= arc.weight || it->weight >= it2->weight) continue;
//                        }
//                        tseen[pt] = true;
//                        wtrans.push_back(pt);
//                    }
//                }
//            } else {
//                for (auto pt : place.producers) {
//                    if (!tseen[pt]) {
//                        // Summary of block: pt is seen unless it forms a non-increasing loop on place
//                        Transition &trans = parent->_transitions[pt];
//                        auto it = trans.pre.begin();
//                        for (; it != trans.pre.end(); ++it)
//                            if (it->place >= arc.place) break;
//
//                        if (it != trans.pre.end() && it->place == arc.place && !it->inhib) {
//                            auto it2 = trans.post.begin();
//                            for (; it2 != trans.post.end(); ++it2)
//                                if (it2->place >= arc.place) break;
//                            if (it->weight >= it2->weight) continue;
//                        }
//                        tseen[pt] = true;
//                        wtrans.push_back(pt);
//                    }
//                }
//
//                for (auto pt : place.consumers) {
//                    if (!tseen[pt] && (!remove_consumers || placeInQuery[arc.place] > 0)) {
//                        tseen[pt] = true;
//                        wtrans.push_back(pt);
//                    }
//                }
//            }
//            pseen[arc.place] = true;
//        }
//    }
//    return std::make_optional(std::pair(tseen, pseen));
//}