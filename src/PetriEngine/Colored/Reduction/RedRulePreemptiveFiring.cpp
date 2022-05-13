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
            if (map.empty()) {
                std::cout << "------------------------------" << std::endl;
                std::cout << "empty map for place: " << *place.name << std::endl;
                continue;
            }

            const Transition &transition = red.transitions()[place._post[0]];
            std::cout << "------------------------------" << std::endl;
            std::cout << "place: " << *place.name << std::endl;
            std::cout << "transition: " << *transition.name << std::endl;
            std::cout << "map: " << std::endl;
            for (auto &mapping: map) {
                auto &otherplace = red.places()[mapping.first];
                std::cout << *otherplace.name << " : " << mapping.second << std::endl;
            }

            for (auto &out: transition.output_arcs) {
                auto &otherplace = const_cast<Place &>(red.places()[out.place]);
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
        Colored::PartitionBuilder partition(red.transitions(), red.places());

        //If there is a guard, and only one color, we good to go
        std::string postColor;
        if (transition.guard) {
            postColor = getTheValidColor(partition, red, transition, p);
        }


        auto place = red.places()[p];
        const auto &in = red.getInArc(p, transition);
        uint32_t canFiresTimes = numFirable(place.marking, *in, transition, partition, red.colors());
        if (canFiresTimes == 0) return pMap;

        // Check if the transition is currently inhibited
        for (auto &inhibArc: red.inhibitorArcs()) {
            if (inhibArc.place == p && inhibArc.transition == t) {
                if (inhibArc.inhib_weight <= place.marking.size()) {
                    return pMap;
                }
            }
        }

        //Could relax this, and only move some tokens, or check distinct size on marking
        if ((place.marking.size() % in->expr->weight()) != 0) {
            return pMap;
        }

        //could perhaps also relax this, but seems much more difficult
        if (transition.input_arcs.size() > 1) return pMap;

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
            if (same) {
                pMap.insert({out.place, "same"});
                continue;
            }
            //for fireability consistency. We don't want to put tokens to a place enabling transition
            std::string validColor;
            for (auto tin: outPlace._post) {
                if (inQuery.isTransitionUsed(tin)) {
                    return emptyMap;
                }
                const Transition &innerTransition = red.transitions()[tin];
                std::string nextValidColor = getTheValidColor(partition, red, innerTransition, out.place);
                if (validColor.empty()) {
                    validColor = nextValidColor;
                }
                if ((transition.guard && (validColor != postColor)) || (nextValidColor != validColor)) {
                    return emptyMap;
                }
            }
            pMap.insert({out.place, validColor});
        }
        return pMap;
    }

    uint32_t RedRulePreemptiveFiring::numFirable(Multiset &marking, const Arc &arc,
                                                 const Colored::Transition &transition,
                                                 PartitionBuilder &partition,
                                                 const ColorTypeMap &colors) const {
        assert(arc.input);
        uint32_t numFire = 0;
        NaiveBindingGenerator gen(transition, colors);
        for (const auto &binding: gen) {
            const ExpressionContext context{binding, colors, partition.partition()[arc.place]};
            const auto ms = EvaluationVisitor::evaluate(*arc.expr, context);
            if ((ms.isSubsetOrEqTo(marking))) numFire++;
        }
        return numFire;
    }

    std::optional<const Color *> RedRulePreemptiveFiring::getTheValidColor(PartitionBuilder &partition, ColoredReducer &red,
                                                          const Colored::Transition &transition, uint32_t p) {
        NaiveBindingGenerator gen(transition, red.colors());
        return std::nullopt;
        //use if() to check if it is null, and dereference to use
        if (!transition.guard) return "";

        bool multipleValid = false;
        const Color * validCol;
        for (auto &binding: gen) {
            const ExpressionContext context{binding, red.colors(), partition.partition()[p]};
            if (EvaluationVisitor::evaluate(*transition.guard, context)) {
                if (multipleValid) {
                    return "";
                }
                multipleValid = true;

                for (auto col: binding) {
                    validCol = col.second;
                }
            }
        }
        return validCol;
    }
}