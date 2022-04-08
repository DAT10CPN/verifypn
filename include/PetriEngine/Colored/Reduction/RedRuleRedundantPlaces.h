//
// Created by mathi on 08/04/2022.
//

#ifndef VERIFYPN_REDRULEREDUNDANTPLACES_H
#define VERIFYPN_REDRULEREDUNDANTPLACES_H


#include "ReductionRule.h"

namespace PetriEngine::Colored::Reduction {
    class RedRuleRedundantPlaces : public ReductionRule {
    public:
        std::string name() override { return "RedundantPlaces"; }

        bool isApplicable(QueryType queryType, bool preserveLoops, bool preserveStutter) const override { return true; }

        bool apply(ColoredReducer &red, const std::vector<bool> &inQuery, QueryType queryType, bool preserveLoops, bool preserveStutter) override;
    };
}


#endif //VERIFYPN_REDRULEREDUNDANTPLACES_H
