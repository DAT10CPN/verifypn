//
// Created by mathi on 04/03/2022.
//

#ifndef VERIFYPN_TESTRULE_H
#define VERIFYPN_TESTRULE_H

#include "ReductionRule.h"

namespace PetriEngine::Colored::Reduction {
    class TestRule : public ReductionRule {
    public:
        std::string name() override { return "Test"; }

        bool canBeAppliedRepeatedly() override { return false; }

        bool apply(ColoredReducer &red, const std::vector<bool> &inQuery, bool preserveDeadlocks) override;
    };
}


#endif //VERIFYPN_TESTRULE_H




