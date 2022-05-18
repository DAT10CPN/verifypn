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

            auto placeColorMap = t_is_viable_and_get_map(red, inQuery, place._post[0], p);
            const Transition &transition = red.transitions()[place._post[0]];
            if (!placeColorMap) {
                std::cout << "------------------------------" << std::endl;
                std::cout << "Not viable" << std::endl;
                std::cout << "place: " << *place.name << std::endl;
                std::cout << "transition: " << *transition.name << std::endl;
                continue;
            }


            if (placeColorMap->empty()) { //&&to_string(*transition.output_arcs[0].expr) != to_string(*transition.input_arcs[0].expr)) {
                std::cout << "------------------------------" << std::endl;
                std::cout << "empty map for place, just copy the color: " << *place.name << std::endl;
                std::cout << "transition: " << *transition.name << std::endl;
                continue;
            }


            std::cout << "------------------------------" << std::endl;
            std::cout << "place: " << *place.name << std::endl;
            std::cout << "transition: " << *transition.name << std::endl;
            std::cout << "map: " << std::endl;
            for (auto &mapping: *placeColorMap) {
                const Color *col = *mapping.second;
                auto &otherplace = red.places()[mapping.first];
                if (mapping.second) {
                    std::cout << *otherplace.name << " : " << col->getColorName() << std::endl;
                } else {
                    std::cout << *otherplace.name << " : anything" << std::endl;
                }
            }

            for (auto &out: transition.output_arcs) {
                std::optional<const Color*> col = placeColorMap->find(out.place)->second;
                if (col) {
                    std::cout << "------------------------------" << std::endl;
                    std::cout << "place: " << out.place << std::endl;
                    std::cout << "color: " << *col->getColorName() << std::endl;
                }
                auto &otherplace = const_cast<Place &>(red.places()[out.place]);
                if (to_string(*out.expr) == to_string(*transition.input_arcs[0].expr)) {
                    std::cout << "same" << std::endl;
                    otherplace.marking += place.marking;
                    continue;
                }
            }
            //place.marking *= 0;


            _applications++;
            continueReductions = true;

        }
        return false;
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

    std::optional<std::map<uint32_t, std::optional<const Color *>>>
    RedRulePreemptiveFiring::t_is_viable_and_get_map(ColoredReducer &red,
                                                     const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                                     uint32_t t, uint32_t p) {


        //fireability consistency check
        if (inQuery.isTransitionUsed(t)) return std::nullopt;

        const Transition &transition = red.transitions()[t];
        Colored::PartitionBuilder partition(red.transitions(), red.places());

        //could perhaps also relax this, but seems much more difficult
        if (transition.input_arcs.size() > 1) return std::nullopt;

        // - Make sure that we do not produce tokens to something that can produce tokens to our preset. To disallow infinite use of this rule by looping
        std::set<uint32_t> already_checked;
        if ((transition_can_produce_to_place(t, p, red, already_checked))) return std::nullopt;

        auto outerPlace = red.places()[p];
        // Check if the transition is currently inhibited
        for (auto &inhibArc: red.inhibitorArcs()) {
            if (inhibArc.place == p && inhibArc.transition == t) {
                if (inhibArc.inhib_weight <= outerPlace.marking.size()) {
                    return std::nullopt;
                }
            }
        }

        const auto &outerIn = red.getInArc(p, transition);
        //Can relax this and see if the actual binding we need allows, and just use that as many times
        //todo fix, does not work for P2
        if (!everyBindingAllows(outerPlace.marking, *outerIn, transition, partition, red.colors())) return std::nullopt;

        //Could relax this, and only move some tokens, or check distinct size on marking
        if ((outerPlace.marking.size() % outerIn->expr->weight()) != 0) {
            return std::nullopt;
        }

        //If there is a guard, and only one color, we good to go
        std::optional<const Color *> singleValidColor;
        std::map<uint32_t, std::optional<const Color *>> pMap;
        if (transition.guard) {
            singleValidColor = getTheValidColor(partition, red, transition, p);
            if (singleValidColor) {
                pMap.insert({p, singleValidColor});
            }
        }

        std::map<uint32_t, std::optional<const Color *>> emptyMap;

        //The inner check
        // - postset cannot inhibit or be in query
        for (auto &out: transition.output_arcs) {
            Place innerPlace = red.places()[out.place];
            if (inQuery.isPlaceUsed(out.place) || innerPlace.inhibitor) {
                return std::nullopt;
            }

            //for fireability consistency. We don't want to put tokens to a place enabling transition
            std::optional<const Color *> validColor;
            for (auto tin: innerPlace._post) {
                if (inQuery.isTransitionUsed(tin)) {
                    return std::nullopt;
                }

                if (singleValidColor) {
                    continue;
                }

                //const auto &innerOut = red.getOutArc(transition, p);

                const Transition &innerTransition = red.transitions()[tin];
                std::optional<const Color *> nextValidColor = getTheValidColor(partition, red, innerTransition,
                                                                               out.place);
                if (!nextValidColor) {
                    return std::nullopt;
                }
                if (!validColor) {
                    validColor = nextValidColor;
                }
                if (nextValidColor != validColor) {
                    return std::nullopt;
                }
            }
            /*if (singleValidColor || !validColor) {
                continue;
            }*/
            pMap.insert({out.place, validColor});
        }
        if (singleValidColor) {
            return emptyMap;
        }
        return pMap;
    }

    bool RedRulePreemptiveFiring::everyBindingAllows(Multiset &marking, const Arc &arc,
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

    std::optional<const Color *>
    RedRulePreemptiveFiring::getTheValidColor(PartitionBuilder &partition, ColoredReducer &red,
                                              const Colored::Transition &transition, uint32_t p) {


        NaiveBindingGenerator gen(transition, red.colors());
        //use if() to check if it is null, and dereference to use
        if (!transition.guard) return std::nullopt;

        bool multipleValid = false;
        const Color *validCol;
        for (auto &binding: gen) {
            const ExpressionContext context{binding, red.colors(), partition.partition()[p]};
            if (EvaluationVisitor::evaluate(*transition.guard, context)) {
                if (multipleValid) {
                    return std::nullopt;
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