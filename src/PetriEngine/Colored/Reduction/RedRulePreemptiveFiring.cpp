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
#include "PetriEngine/Colored/BindingGenerator.h"

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
            if (place.marking.empty()) continue;
            if (inQuery.isPlaceUsed(p)) continue;

            // Must be exactly one post, in order to not remove branching
            if (place._post.size() != 1) {
                continue;
            }

            auto map = t_is_viable_and_get_map(red, inQuery, place._post[0], p);
            if (map.empty()) continue;

            const Transition &transition = red.transitions()[place._post[0]];
            std::cout << "place: " << *place.name << std::endl;
            std::cout << "transition: " << *transition.name << std::endl;
            std::cout << "map: " << std::endl;
            for (auto &mapping: map) {
                std::cout << mapping.first << " : " << mapping.second << std::endl;
            }

            for (auto &out: transition.output_arcs) {
                auto &otherplace = const_cast<Place &>(red.places()[out.place]);
                std::cout << "otherplace: " << *otherplace.name << std::endl;
                //otherplace.marking += place.marking;
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

    std::map<uint32_t, std::string>
    RedRulePreemptiveFiring::t_is_viable_and_get_map(ColoredReducer &red,
                                                     const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                                     uint32_t t, uint32_t p) {

        std::map<uint32_t, std::string> pMap;

        //fireability consistency check
        if (inQuery.isTransitionUsed(t)) return pMap;

        const Transition &transition = red.transitions()[t];
        // Easiest to not handle guards, todo if guard, iterate through bindings and find the valid bindings, and only move those tokens
        auto place = red.places()[p];
        Colored::PartitionBuilder partition(red.transitions(), red.places());
        const auto &in = red.getInArc(p, transition);

        //Guard check was here before
        if (!markingEnablesInArc(place.marking, *in, transition, partition, red.colors())) return pMap;

        // Check if the transition is currently inhibited
        for (auto &inhibArc: red.inhibitorArcs()) {
            if (inhibArc.place == p && inhibArc.transition == t) {
                if (inhibArc.inhib_weight <= place.marking.size()) {
                    return pMap;
                }
            }
        }

        //could perhaps also relax this, but seems much more difficult
        if (transition.input_arcs.size() > 1) return pMap;

        //Could relax this, and only move some tokens, or check distinct size on marking

        if ((place.marking.size() % in->expr->weight()) != 0) {
            return pMap;
        }

        // - Make sure that we do not produce tokens to something that can produce tokens to our preset. To disallow infinite use of this rule by looping
        std::set<uint32_t> already_checked;
        if ((transition_can_produce_to_place(t, p, red, already_checked))) return pMap;

        std::map<uint32_t, std::string> emptyMap;
        // - postset cannot inhibit or be in query
        for (auto &out: transition.output_arcs) {
            Place outPlace = red.places()[out.place];
            if (inQuery.isPlaceUsed(out.place) || outPlace.inhibitor) {
                return emptyMap;
            }

            bool same = to_string(*out.expr) == to_string(*in->expr);
            //for fireability consistency. We don't want to put tokens to a place enabling transition
            for (auto tin: outPlace._post) {
                if (inQuery.isTransitionUsed(tin)) {
                    return emptyMap;
                }
                if (same) {
                    pMap.insert({out.place, "same"});
                }

                pMap.insert({out.place, getTheValidColor(partition, red, out,
                                                         tin, transition)});

            }
        }
        return pMap;
    }

    bool RedRulePreemptiveFiring::markingEnablesInArc(Multiset &marking, const Arc &arc,
                                                      const Colored::Transition &transition,
                                                      PartitionBuilder &partition,
                                                      const ColorTypeMap &colors) const {
        assert(arc.input);

        NaiveBindingGenerator gen(transition, colors);
        for (const auto &binding: gen) {
            const ExpressionContext context{binding, colors, partition.partition()[arc.place]};
            const auto ms = EvaluationVisitor::evaluate(*arc.expr, context);
            if (!(ms.isSubsetOrEqTo(marking))) return false;
        }
        return true;
    }

    std::string
    RedRulePreemptiveFiring::getTheValidColor(PartitionBuilder &partition, ColoredReducer &red, const Arc &arc,
                                              uint32_t tin, const Colored::Transition &transition) {
        const Transition &innerTransition = red.transitions()[tin];
        NaiveBindingGenerator gen(transition, red.colors());


        bool multipleValid = false;
        for (auto &binding: gen) {

            const ExpressionContext context{binding, red.colors(), partition.partition()[arc.place]};
            if (EvaluationVisitor::evaluate(*innerTransition.guard, context)) {
                if (multipleValid) {
                    return "";
                }
                multipleValid = true;

                for (auto col: binding) {
                    return col.second->getColorName();
                }
            }
        }
        return "";
    }
}