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
            if (place.marking.empty()) continue;
            if (inQuery.isPlaceUsed(p)) continue;

            // Must be exactly one post, in order to not remove branching
            if (place._post.size() != 1) {
                continue;
            }

            if (!t_is_viable(red, inQuery, place._post[0], p)) continue;

            const Transition &transition = red.transitions()[place._post[0]];

            for (auto &out: transition.output_arcs) {
                std::cout << "transition: " << *transition.name << std::endl;
                std::cout << "place: " << *place.name << std::endl;
                auto &otherplace = const_cast<Place &>(red.places()[out.place]);
                std::cout << "otherplace: " << *otherplace.name << std::endl;
                otherplace.marking += place.marking;
            }
            place.marking *= 0;


            _applications++;
            continueReductions = true;

        }
        return continueReductions;
    }

    // Search function to see if a transition t can somehow get tokens to place p. Very overestimation, just looking at arcs
    bool RedRulePreemptiveFiring::transition_can_produce_to_place(unsigned int t, uint32_t p, ColoredReducer &red,
                                                                  std::set<uint32_t> &already_checked) const {
        const Transition &transition = red.transitions()[t];
        for (auto &out: transition.output_arcs) {

            //base case
            if (out.place == p) {
                return true;
            }

            if (already_checked.find(out.place) != already_checked.end()) {
                continue;
            } else {
                already_checked.insert(out.place);
            }

            const Place &place = red.places()[out.place];
            if (place.skipped) continue;

            // recursive case
            for (auto &inout: place._post) {
                bool can_produce = (transition_can_produce_to_place(inout, p, red, already_checked));
                if (can_produce) return true;
            }
        }

        return false;
    }

    bool RedRulePreemptiveFiring::t_is_viable(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                              uint32_t t, uint32_t p) {
        //fireability consistency check
        if (inQuery.isTransitionUsed(t)) return false;

        const Transition &transition = red.transitions()[t];
        // Easiest to not handle guards, todo if guard, iterate through bindings and find the valid bindings, and only move those tokens
        //todo Inhib is actually ok, but we need to check if it is inhibited in the current marking
        if (transition.guard || transition.inhibited) return false;

        //could perhaps also relax this, but seems much more difficult
        if (transition.input_arcs.size() > 1) return false;

        // - Make sure that we do not produce tokens to something that can produce tokens to our preset. To disallow infinite use of this rule by looping
        std::set<uint32_t> already_checked;
        if ((transition_can_produce_to_place(t, p, red, already_checked))) return false;

        bool ok = true;
        // - postset cannot inhibit or be in query
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

            //todo could relax this, and instead of simply copying the tokens to the new place, then update them according to the out arc expression
            const auto &in = red.getInArc(p, transition);
            if (to_string(*out.expr) != to_string(*in->expr)) {
                ok = false;
                break;
            }
        }

        if (!ok) return false;

        return true;
    }
}