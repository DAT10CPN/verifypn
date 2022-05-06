/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#ifndef VERIFYPN_REDRULEPREEMPTIVEFIRING_H
#define VERIFYPN_REDRULEPREEMPTIVEFIRING_H

#include "ReductionRule.h"
#include "PetriEngine/Colored/ColoredNetStructures.h"

namespace PetriEngine::Colored::Reduction {
    class RedRulePreemptiveFiring : public ReductionRule {
    public:
        std::string name() override { return "PreemptiveFiring"; }

        bool isApplicable(QueryType queryType, bool preserveLoops, bool preserveStutter) const override {
            return true; //todo queryType == QueryType::Reach && !preserveStutter && !preserveLoops;
        }

        bool apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery, QueryType queryType,
                   bool preserveLoops, bool preserveStutter) override;

        std::pair<bool, std::vector<uint32_t>> transition_can_produce_to_place(unsigned int t, uint32_t p, ColoredReducer &red, std::vector<uint32_t> already_checked) const;
    };
}


#endif //VERIFYPN_REDRULEPREEMPTIVEFIRING_H
