//
// Created by nicoesterby on 23-02-2022.
//

#include "PetriEngine/Colored/Reduction/ReduceFirstPlace.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool ReduceFirstPlace::apply(ColoredReducer& red, const std::vector<bool> &in_query,
                                 bool can_remove_deadlocks) {

        // Remove place 0 - who cares about correctness :)
        if (!red.places().empty() && !red.places()[0].skipped && !in_query[0]) {
            red.skipPlace(0);
            _applications++;
            return true;
        }

        return false;
    }
}