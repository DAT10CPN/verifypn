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
            for (uint cons: place._pre) {
                Transition t = red.transitions()[cons];
                auto w = red.getInArc(p, t)->expr->weight();
                if (w > place.marking.size()) {
                    ok = false;
                    break;
                } else {
                    auto it = red.getOutArc(t, p);
                    if (it == t.output_arcs.end() ||
                        it->place != p ||
                        it->expr->weight() < w) {
                        ok = false;
                        break;
                    }
                }
            }

            if (!ok) continue;

            ++_applications;

            if (red.unskippedPlacesCount() > 1) {
                red.skipPlace(p);
                continueReductions = true;
            }
        }
        red.consistent();
        return continueReductions;
    }
}