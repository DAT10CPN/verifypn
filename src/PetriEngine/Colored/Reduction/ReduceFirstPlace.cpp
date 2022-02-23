//
// Created by nicoesterby on 23-02-2022.
//

#include "PetriEngine/Colored/Reduction/ReduceFirstPlace.h"

namespace PetriEngine::Colored::Reduction {
    bool ReduceFirstPlace::apply(const ColoredReducer& red, const std::vector<bool> &in_query,
                                 bool can_remove_deadlocks) {
        return false;
    }
}