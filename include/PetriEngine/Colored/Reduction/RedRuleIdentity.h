/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#ifndef VERIFYPN_REDRULEIDENTITY_H
#define VERIFYPN_REDRULEIDENTITY_H

#include "ReductionRule.h"

namespace PetriEngine::Colored::Reduction {
    class RedRuleIdentity : public ReductionRule {
    public:
        std::string name() override { return "Identity"; }

        bool canBeAppliedRepeatedly() override { return false; }

        bool apply(ColoredReducer &red, const std::vector<bool> &inQuery, QueryType queryType, bool preserveLoops, bool preserveStutter, bool onlyReach, uint32_t explosion_limiter) override;
    };
}

#endif //VERIFYPN_REDRULEIDENTITY_H
