//
// Created by Nicoe on 23-02-2022.
//

#ifndef VERIFYPN_REDUCEFIRSTPLACE_H
#define VERIFYPN_REDUCEFIRSTPLACE_H

#include "ReductionRule.h"

namespace PetriEngine::Colored::Reduction {
    class ReduceFirstPlace : public ReductionRule {
    public:
        bool apply(ColoredReducer& red, const std::vector<bool> &in_query, bool can_remove_deadlocks) override;
        bool canBeAppliedRepeatedly() override { return false; }
    };
}

#endif //VERIFYPN_REDUCEFIRSTPLACE_H
