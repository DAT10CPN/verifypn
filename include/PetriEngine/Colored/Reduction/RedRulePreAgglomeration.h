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
    class RedRulePreAgglomeration : public ReductionRule {
    public:
        std::string name() override { return "Atomic Pre-Agglomeration (Colored)"; }

        bool canBeAppliedRepeatedly() override { return false; }

        bool apply(ColoredReducer &red, const std::vector<bool> &inQuery, bool preserveDeadlocks, bool onlyReach, uint32_t explosion_limiter) override;
    };
}

#endif //VERIFYPN_REDRULEIDENTITY_H
