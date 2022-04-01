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
        return !preserveStutter;
    }

    bool RedRuleRelevance::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                        QueryType queryType, bool preserveLoops, bool preserveStutter) {

        bool changed = false;
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
                    for (uint32_t prtID : place._pre) {
                        if (!tseen[prtID]) {
                            const PetriEngine::Colored::Transition& potentiallyRelevantTrans = red.transitions()[prtID];
                            // Loops that do not alter the marking in the place are not considered relevant to the place.
                            auto prtIn = potentiallyRelevantTrans.input_arcs.begin();
                            for (; prtIn != potentiallyRelevantTrans.input_arcs.end(); ++prtIn){
                                if (prtIn->place >= arc.place) break;
                            }
                            if (prtIn != potentiallyRelevantTrans.input_arcs.end() && prtIn->place == arc.place) {
                                const auto& prtOut = red.getOutArc(potentiallyRelevantTrans, arc.place);
                                if (const auto ms1 = PetriEngine::Colored::extractVarMultiset(*prtIn->expr)){
                                    if (const auto ms2 = PetriEngine::Colored::extractVarMultiset(*prtIn->expr)) {
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
                    for (const auto prtID : place._post) {
                        if (!tseen[prtID]) {
                            // Summary of block: the potentially relevant transition is seen unless it:
                            // - Is inhibited by 'place'
                            // - Forms a decreasing loop on 'place' that cannot lower the token count of 'place' below the weight of 'arc'
                            // - Forms a non-decreasing loop on 'place'
                            const Transition &potentiallyRelevantTrans = red.transitions()[prtID];
                            auto prtOut = potentiallyRelevantTrans.output_arcs.begin();
                            for (; prtOut != potentiallyRelevantTrans.output_arcs.end(); ++prtOut)
                                if (prtOut->place >= arc.place) break;

                            if (prtOut != potentiallyRelevantTrans.output_arcs.end() && prtOut->place == arc.place) {
                                const auto prtIn = red.getInArc(arc.place, potentiallyRelevantTrans);
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

        for (uint32_t i = red.placeCount(); i > 0;) {
            // This loop structure avoids underflow handling while also minimizing placeCount() calls.
            i--;
            if (pseen[i] || red.places()[i].skipped){
                continue;
            } else {
                red.skipPlace(i);
                changed |= true;
            }
        }
        for (uint32_t i = red.transitionCount(); i > 0;) {
            // This loop structure avoids underflow handling while also minimizing transitionCount() calls.
            i--;
            if (tseen[i] || red.transitions()[i].skipped){
                continue;
            } else {
                red.skipTransition(i);
                changed |= true;
            }
        }
        if (changed) _applications++;
        return changed;
    }
}