/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */
#include <PetriEngine/Colored/PartitionBuilder.h>
#include <PetriEngine/Colored/EvaluationVisitor.h>
#include "PetriEngine/Colored/Reduction/RedRulePreemptiveFiring.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/Colored/ArcVarMultisetVisitor.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreemptiveFiring::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                        QueryType queryType,
                                        bool preserveLoops, bool preserveStutter) {
        Colored::PartitionBuilder partition(red.transitions(), red.places());
        bool continueReductions = false;
        const size_t numberofplaces = red.placeCount();
        for (uint32_t p = 0; p < numberofplaces; ++p) {

            if (red.hasTimedOut()) return false;
            auto &place = const_cast<Place &>(red.places()[p]);
            if (place.skipped) continue;
            if (place.inhibitor) continue; //todo can do more, but eh
            if (place.marking.empty()) continue;

            // - Preset can only have one transition in post
            if (place._post.size() != 1) continue;
            //fireability consistency check
            if (inQuery.isTransitionUsed(place._post[0])) continue;

            bool ok = true;
            // - Make sure that we do not produce tokens to something that can produce tokens to our preset. To disallow infinite use of this rule
            if ((transition_can_produce_to_place(place._post[0], p, red, std::vector<uint32_t>())).first) continue;

            const Transition &transition = red.transitions()[place._post[0]];

            // Easiest to not handle guards, if guard, iterate through bindings
            if (transition.guard) continue;

            if (transition.input_arcs.size() > 1) continue; //could also do something here?
            const auto &in = red.getInArc(p, transition);

            // - Preset and postset cannot inhibit or be in query
            for (auto &out: transition.output_arcs) {
                if (inQuery.isPlaceUsed(out.place) || red.places()[out.place].inhibitor) {
                    ok = false;
                    break;
                }
                //for fireability consistency. We don't want to put tokens to a place enabling transition
                for (auto &tin: red.places()[out.place]._post) {
                    if (inQuery.isTransitionUsed(tin)) {
                        ok = false;
                        break;
                    }
                }


                //relax this
                if (to_string(*out.expr) != to_string(*in->expr)) {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            // - Preset and postset cannot inhibit or be in query
            for (auto &in_arc: transition.input_arcs) {
                if (inQuery.isPlaceUsed(in_arc.place) || red.places()[in_arc.place].inhibitor) {
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


            _applications++;
            continueReductions = true;

        }
        return continueReductions;
    }

    std::pair<bool, std::vector<uint32_t>>
    RedRulePreemptiveFiring::transition_can_produce_to_place(unsigned int t, uint32_t p, ColoredReducer &red,
                                                             std::vector<uint32_t> already_checked) const {
        const Transition &transition = red.transitions()[t];
        for (auto &out: transition.output_arcs) {

            if (std::find(already_checked.begin(), already_checked.end(), out.place) != already_checked.end()) {
                continue;
            } else {
                already_checked.push_back(out.place);
            }

            const Place &place = red.places()[out.place];
            if (place.skipped) continue;

            //base case
            if (out.place == p) {
                return std::pair(true, already_checked);
                /*const auto &inArc = red.getInArc(p, transition);
                if (inArc == transition.input_arcs.end()) continue;
                auto inSet = PetriEngine::Colored::extractVarMultiset(*inArc->expr);
                auto outSet = PetriEngine::Colored::extractVarMultiset(*out.expr);
                if (!inSet || (inSet && (*inSet).isSubsetOrEqTo(*outSet))) {
                    return std::pair(true, already_checked);
                }*/
            }

            // recursive case
            for (auto &inout: place._post) {
                auto res = (transition_can_produce_to_place(inout, p, red, already_checked));
                if (res.first) return std::pair(true, already_checked);
                already_checked = res.second;
            }
        }

        return std::pair(false, already_checked);
    }
}