//
// Created by mathi on 21/04/2022.
//

#include "PetriEngine/Colored/Reduction/RedRulePreemptiveFiring.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRulePreemptiveFiring::apply(ColoredReducer &red, const PetriEngine::PQL::ColoredUseVisitor &inQuery,
                                       QueryType queryType, bool preserveLoops, bool preserveStutter) {
        // Fire initially enabled transitions if they are the single consumer of their preset
        bool continueReductions = false;

        for (uint32_t t = 0; t < red.transitionCount(); ++t)
        {
            const Transition& tran = red.transitions()[t];

            if (tran.skipped || tran.inhibited || tran.input_arcs.empty()) continue;


            // - Preset and postset must be disjoint (to avoid immediate infinite use)
            // - Preset and postset cannot inhibit or be in query
            // scale marking
            // - Preset can only have this transition in postset
            // - Type of places in postset is exactly that of the place in preset
            // - Only one variable on arc
            // Easiest to not handle guards, if guard, iterate through all bindings.

            _applications++;
            continueReductions = true;
        }

        return continueReductions;
    }
}