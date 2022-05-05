/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRulePreemptiveFiring.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/Colored/ArcVarMultisetVisitor.h"

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
            if (place.inhibitor) continue; //todo can do more, but eh
            if (place.marking.empty()) continue;
            if (place._post.size() != 1)
                continue; // could do something better, could have more transitions, but not all are enabled
            //if (!place._pre.empty()) continue;

            bool ok = true;
            if (transition_can_produce_to_place(place._post[0], p, red, std::vector<uint32_t> ())) continue;
            const Transition &transition = red.transitions()[place._post[0]];
            if (transition.guard) continue; //could perhaps do something
            if (transition.input_arcs.size() > 1) continue; //could also do something here?
            const auto &in = red.getInArc(p, transition);
            if (inQuery.isPlaceUsed(in->place)) continue; // later do some more fancy where t can have more input arcs

            for (auto &out: transition.output_arcs) {
                if (inQuery.isPlaceUsed(out.place)) {
                    ok = false;
                    break;
                }

                if (to_string(*out.expr) != to_string(*in->expr)) {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            for (auto &in_arcs: transition.input_arcs) {
                if (inQuery.isPlaceUsed(in_arcs.place)) {
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
            continueReductions = true;

        }
        return continueReductions;
    }

    bool RedRulePreemptiveFiring::transition_can_produce_to_place(unsigned int t, uint32_t p, ColoredReducer &red, std::vector<uint32_t> already_checked) const {
        const Transition &transition = red.transitions()[t];
        for (auto &out: transition.output_arcs) {

            if(std::find(already_checked.begin(), already_checked.end(), out.place) != already_checked.end()) {
                continue;
            } else {
                already_checked.push_back(out.place);
            }

            const Place &place = red.places()[out.place];
            if (place.skipped) continue;
            //base case
            if (out.place == p) {
                const auto &inArc = red.getInArc(p, transition);
                if (inArc == transition.input_arcs.end()) continue;
                auto inSet = PetriEngine::Colored::extractVarMultiset(*inArc->expr);
                auto outSet = PetriEngine::Colored::extractVarMultiset(*out.expr);
                if (!inSet || (inSet && (*inSet).isSubsetOrEqTo(*outSet))) {
                    return true;
                }
            }

            // recursive case
            for (auto &inout: place._post) {
                if (transition_can_produce_to_place(inout, p, red, already_checked)) return true;
            }
        }

        return false;
    }
}