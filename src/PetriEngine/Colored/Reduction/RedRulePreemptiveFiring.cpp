//
// Created by mathi on 21/04/2022.
//

#include "PetriEngine/Colored/Reduction/RedRulePreemptiveFiring.h"
#include <PetriEngine/Colored/ArcVarMultisetVisitor.h>
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreemptiveFiring::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                       QueryType queryType, bool preserveLoops, bool preserveStutter) {
        // Fire initially enabled transitions if they are the single consumer of their preset

        bool continueReductions = false;

        for (uint32_t t = 0; t < red.transitionCount(); ++t)
        {
            const Transition& tran = red.transitions()[t];

            if (tran.skipped || tran.inhibited || tran.input_arcs.empty() || tran.output_arcs.empty()) continue;
            // - Preset and postset must be disjoint (to avoid infinite use)

            // - Preset and postset cannot inhibit or be in query

            // - Preset can only have this transition in postset

            // - How many times can we fire the transition



            _applications++;
            continueReductions = true;
        }

        return continueReductions;
    }
}