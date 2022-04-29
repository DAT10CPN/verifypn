/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRulePreemptiveFiring.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreemptiveFiring::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                        QueryType queryType,
                                        bool preserveLoops, bool preserveStutter) {
        bool continueReductions = false;
        const size_t numberofplaces = red.placeCount();
        for (uint32_t p = 0; p < numberofplaces; ++p) {

            if (red.hasTimedOut()) return false;
            auto &place = const_cast<Place &>(red.places()[p]);
            if (place.skipped) continue;
            if (inQuery.isPlaceUsed(p)) continue;
            if (place.inhibitor) continue; //todo can do more
            if (place.marking.empty()) continue;
            if (place._post.size() != 1) continue; // could do something else
            if (!place._pre.empty()) continue;

            bool ok = true;

            Transition transition = red.transitions()[place._post[0]];
            if (transition.guard) continue;
            const auto &in = red.getInArc(p, transition);

            for (auto &out: transition.output_arcs) {
                if (to_string(*out.expr) != to_string(*in->expr)) {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            for (auto &out: transition.output_arcs) {
                auto &otherplace = const_cast<Place &>(red.places()[out.place]);
                otherplace.marking += place.marking;

            }
            place.marking *= 0;

            // - Preset and postset must be disjoint (to avoid immediate infinite use)
            // - Preset and postset cannot inhibit or be in query
            // scale marking
            // - Preset can only have this transition in postset
            // - Type of places in postset is exactly that of the place in preset
            // - Only one variable on arc
            // Easiest to not handle guards, if guard, iterate through all bindings.

            _applications++;
            continueReductions = false;

        }
        return continueReductions;
    }
}