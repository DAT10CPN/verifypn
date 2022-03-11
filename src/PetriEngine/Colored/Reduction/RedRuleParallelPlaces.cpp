/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRuleParallelPlaces.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleParallelPlaces::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                      bool preserveDeadlocks) {

        // Remove places which input and output is k times another place's input and output

        bool continueReductions = false;

        auto &places = red.places();
        auto &transitions = red.transitions();

        red._pflags.resize(places.size());
        std::fill(red._pflags.begin(), red._pflags.end(), 0);

        for (size_t touter = 0; touter < transitions.size(); touter++) {
            for (size_t outer = 0; outer < transitions[touter].output_arcs.size(); outer++) {

                auto pouter = transitions[touter].output_arcs[outer].place;
                if (red._pflags[pouter] > 0) continue;
                red._pflags[pouter] = 1;
                if (red.hasTimedOut()) return false;
                if (places[pouter].skipped || places[pouter].inhibitor) continue;

                for (size_t inner = outer + 1; inner < transitions[touter].output_arcs.size(); inner++) {

                    auto pinner = transitions[touter].output_arcs[inner].place;
                    if (places[pinner].skipped || places[pinner].inhibitor) continue;

                    if (pouter == pinner) continue;
                    if (places[inner].type != places[outer].type) continue;

                    for (size_t swp = 0; swp < 2; swp++) {

                        if (red.hasTimedOut()) return false;
                        if (places[inner].skipped || places[outer].skipped) break;

                        uint32_t p1 = pouter;
                        uint32_t p2 = pinner;
                        if (swp == 1) std::swap(p1, p2);

                        // We will now check if p2 can be removed

                        if (inQuery[p2]) continue;

                        const Place &place1 = places[p1];
                        const Place &place2 = places[p2];

                        if (place1._pre.size() < place2._pre.size() ||
                            place1._post.size() > place2._post.size())
                            continue;

                        // TODO Check if p2 is k times p1 once we have variable multisets

                        bool ok = true;
                        size_t j = 0;
                        for (size_t i = 0; i < place2._post.size(); i++) {

                            // place1 may have consumers that place2 does not
                            while (j < place1._post.size() && place1._post[j] < place2._post[i]) j++;
                            if (place1._post.size() <= j || place1._post[j] != place2._post[i]) {
                                ok = false;
                                break;
                            }

                            const Transition &trans = transitions[place1._post[j]];
                            auto a1 = red.getInArc(p1, trans);
                            auto a2 = red.getInArc(p2, trans);
                            assert(a1 != trans.input_arcs.end());
                            assert(a2 != trans.input_arcs.end());
                            // TODO Check if subseteq instead of equal
                            if (to_string(*a1->expr) != to_string(*a2->expr)) {
                                ok = false;
                                break;
                            }
                        }

                        if (!ok) break;

                        if (place2.marking.isSubsetOf(place1.marking)) continue;

                        j = 0;
                        for (size_t i = 0; i < place1._pre.size(); i++) {

                            // place2 may have producers that place1 does not
                            while (j < place2._pre.size() && place2._pre[j] < place1._pre[i]) j++;
                            if (j == place2._pre.size() || place1._pre[j] != place2._pre[i]) {
                                ok = false;
                                break;
                            }

                            const Transition &trans = transitions[place1._pre[j]];
                            auto a1 = red.getInArc(p1, trans);
                            auto a2 = red.getInArc(p2, trans);
                            assert(a1 != trans.output_arcs.end());
                            assert(a2 != trans.output_arcs.end());
                            // TODO Check if subseteq instead of equal
                            if (to_string(*a1->expr) != to_string(*a2->expr)) {
                                ok = false;
                                break;
                            }
                        }

                        if (!ok) break;

                        // Remove p2
                        _applications++;
                        continueReductions = true;
                        red.skipPlace(p2);
                        red._pflags[pouter] = 0;
                        break;
                    }
                }
            }
        }

        return continueReductions;
    }
}