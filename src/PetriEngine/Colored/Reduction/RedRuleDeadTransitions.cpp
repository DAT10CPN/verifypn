/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include <PetriEngine/Colored/ArcVarMultisetVisitor.h>
#include <PetriEngine/Colored/BindingGenerator.h>
#include <PetriEngine/Colored/EvaluationVisitor.h>
#include <PetriEngine/Colored/VariableVisitor.h>
#include "PetriEngine/Colored/Reduction/RedRuleDeadTransitions.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleDeadTransitions::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                      QueryType queryType, bool preserveLoops, bool preserveStutter) {

        Colored::PartitionBuilder partition(red.transitions(), red.places());
        bool continueReductions = false;
        const size_t numberofplaces = red.placeCount();
        for(uint32_t p = 0; p < numberofplaces; ++p)
        {
            if(red.hasTimedOut()) return false;
            Place place = red.places()[p];
            if(place.skipped) continue;
            if(place.inhibitor) continue;
            if(place._pre.size() > place._post.size()) continue;

            bool ok = true;
            // Check for producers without matching consumers first
            for(uint prod : place._pre)
            {
                // Any producer without a matching consumer blocks this rule
                const Transition &t = red.transitions()[prod];
                auto in = red.getInArc(p, t);
                if(in == t.input_arcs.end())
                {
                    ok = false;
                    break;
                }
            }

            if(!ok) continue;

            std::set<uint32_t> notenabled;
            // Out of the consumers, tally up those that are initially not enabled by place
            // Ensure all the enabled transitions that feed back into place are non-increasing on place.
            for(uint cons : place._post)
            {
                const Transition &t = red.transitions()[cons];

                uint32_t colorSize = getColorSize(t);
                if (colorSize > 10000) continue;

                const auto &in = red.getInArc(p, t);
                if(markingEnablesInArc(place.marking, *in, t, partition, in->expr->getColors(red.colors())))
                {
                    // This branch happening even once means notenabled.size() != consumers.size()
                    auto out = red.getOutArc(t, p);
                    if (out == t.output_arcs.end()) {
                        continue;
                    }
                    // Only increasing loops are not ok
                    auto inSet = PetriEngine::Colored::extractVarMultiset(*in->expr);
                    auto outSet = PetriEngine::Colored::extractVarMultiset(*out->expr);
                    if (!inSet || !outSet || (*inSet).isSubsetOrEqTo(*outSet)) {
                        if (to_string(*out->expr) != to_string(*in->expr)) {
                            ok = false;
                            break;
                        }
                    }
                }
                else
                {
                    notenabled.insert(cons);
                }
            }

            if(!ok || notenabled.empty()) continue;

            bool skipplace = (notenabled.size() == place._pre.size() && !inQuery.isPlaceUsed(p));

            for(uint32_t cons : notenabled) {
                if (inQuery.isTransitionUsed(cons))
                    skipplace = false;
                else
                {
                    red.skipTransition(cons);
                }
            }

            if(skipplace) {
                red.skipPlace(p);
            }

            _applications++;
            continueReductions = true;

        }
        red.consistent();
        return continueReductions;
    }

    bool RedRuleDeadTransitions::markingEnablesInArc(Multiset &marking, const Arc &arc,
                                                     const Colored::Transition &transition,
                                                     PartitionBuilder &partition,
                                                     const ColorTypeMap &colors) const {
        assert(arc.input);
        NaiveBindingGenerator gen(transition, colors);
        for (const auto &binding: gen) {
            const ExpressionContext context{binding, colors, partition.partition()[arc.place]};
            const auto ms = EvaluationVisitor::evaluate(*arc.expr, context);
            if (ms.isSubsetOrEqTo(marking)) return true;
        }
        return false;
    }

    uint32_t RedRuleDeadTransitions::getColorSize(const Transition &transition) {
        std::set<const Colored::Variable*> variables;

        for (const auto &arc : transition.input_arcs) {
            assert(arc.expr != nullptr);
            Colored::VariableVisitor::get_variables(*arc.expr, variables);
        }
        for (const auto &arc : transition.output_arcs) {
            assert(arc.expr != nullptr);
            Colored::VariableVisitor::get_variables(*arc.expr, variables);
        }

        uint32_t size = 0;
        for (auto& v: variables) {
            if (size == 0) {
                size = v->colorType->size();
            } else {
                size *= v->colorType->size();
            }
        }
        return size;
    }
}