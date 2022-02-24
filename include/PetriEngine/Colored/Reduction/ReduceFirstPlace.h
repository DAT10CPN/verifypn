//
// Created by Nicoe on 23-02-2022.
//

#ifndef VERIFYPN_REDUCEFIRSTPLACE_H
#define VERIFYPN_REDUCEFIRSTPLACE_H

#include "ReductionRule.h"

namespace PetriEngine::Colored::Reduction {
    class ReduceFirstPlace : public ReductionRule {
    public:
        std::string name() override { return "0"; }
        bool canBeAppliedRepeatedly() override { return false; }

        bool apply(ColoredReducer& red, const std::vector<bool> &in_query, bool can_remove_deadlocks) override;
    };
}

#endif //VERIFYPN_REDUCEFIRSTPLACE_H
