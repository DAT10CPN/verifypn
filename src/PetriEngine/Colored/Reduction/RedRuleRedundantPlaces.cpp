/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include <PetriEngine/Colored/PartitionBuilder.h>
#include <PetriEngine/Colored/EvaluationVisitor.h>
#include <PetriEngine/Colored/BindingGenerator.h>
#include "PetriEngine/Colored/Reduction/RedRuleRedundantPlaces.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/Colored/ArcVarMultisetVisitor.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleRedundantPlaces::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                       QueryType queryType, bool preserveLoops, bool preserveStutter) {

        Colored::PartitionBuilder partition(red.transitions(), red.places());
        partition.compute(5);

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

                /*if (t.guard) {
                    ok = false;
                    break;
                }*/

                const auto &outArc = red.getOutArc(t, p);
                if (outArc == t.output_arcs.end()) {
                    ok = false;
                    break;
                }

                const auto &inArc = red.getInArc(p, t);

                NaiveBindingGenerator gen(t, red.colors());
                for (const auto &binding: gen) {
                    if (!(this->markingSatisfiesInArc(place.marking, *inArc, red.colors(), partition,
                                                      binding))) {
                        ok = false;
                        break;
                    }
                }

                if (!ok) break;

                //check if we have succ or pred, optional
                if (auto inSet = PetriEngine::Colored::extractVarMultiset(*inArc->expr)) {
                    if (auto outSet = PetriEngine::Colored::extractVarMultiset(*outArc->expr)) {
                        if (!(*inSet).isSubsetOrEqTo(*outSet)) {
                            ok = false;
                            break;
                        }
                    }
                }
            }

            if (!ok) continue;

            if (red.unskippedPlacesCount() > 1) {
                ++_applications;
                red.skipPlace(p);
                continueReductions = true;
            }
        }
        red.consistent();
        return continueReductions;
    }

    bool RedRuleRedundantPlaces::markingSatisfiesInArc(Multiset &marking, const Arc &arc, ColorTypeMap colors,
                                                       PartitionBuilder &partition,
                                                       const Colored::BindingMap &binding) const {
        assert(arc.input);
        const ExpressionContext context{binding, colors, partition.partition()[arc.place]};
        const auto ms = EvaluationVisitor::evaluate(*arc.expr, context);
        return (ms.isSubsetOrEqTo(marking));
    }
}