/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRuleRedundantPlaces.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/Colored/ArcVarMultisetVisitor.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleRedundantPlaces::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                       QueryType queryType, bool preserveLoops, bool preserveStutter) {
        bool continueReductions = false;
        const size_t numberofplaces = red.placeCount();
        for (uint32_t p = 0; p < numberofplaces; ++p) {
            if (red.hasTimedOut()) return false;
            Place place = red.places()[p];
            if (place.skipped) continue;
            if (place.inhibitor) continue;
            if (place._pre.size() < place._post.size()) continue;
            if (inQuery[p] != 0) continue;

            bool ok = true;
            for (uint cons: place._post) {
                const Transition &t = red.transitions()[cons];

                if (t.guard) {
                    ok = false;
                    break;
                }

                const auto &inArc = red.getInArc(p, t);

                //check if initial marking allows to fire the transition once
                //this is very very mvp. InArc expr can also be all. And can be more complex
                if (!(place.marking.satisfies(*inArc))) {
                    ok = false;
                    break;
                } else {
                    const auto &outArc = red.getOutArc(t, p);
                    if (outArc == t.output_arcs.end()) {
                        ok = false;
                        break;
                    }

                    if (auto inSet = PetriEngine::Colored::extractVarMultiset(*inArc->expr)) {
                        if (auto outSet = PetriEngine::Colored::extractVarMultiset(*outArc->expr)) {
                            if (!(*inSet).isSubsetOrEqTo(*outSet)) {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (!ok) continue;

            if (red.unskippedPlacesCount() > 1) {
                ++_applications;
                std::cout << place.name << std::endl;
                red.skipPlace(p);
                continueReductions = true;
            }
        }
        red.consistent();
        return continueReductions;
    }
}