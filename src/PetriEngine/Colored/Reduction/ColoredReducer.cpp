//
// Created by nicoesterby on 22-02-2022.
//

#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {

    bool ColoredReducer::reduce(uint32_t timeout, const std::vector<bool>& in_query, bool can_remove_deadlocks) {
        if (timeout <= 0) return false;

        auto start_time = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();

        bool changed;
        do {
            changed = false;

            for (auto& rule : _reductions) {
                if (rule->canBeAppliedRepeatedly())
                    while (rule->apply(*this, in_query, can_remove_deadlocks)) changed = true;
                else
                    changed |= rule->apply(*this, in_query, can_remove_deadlocks);
            }

            now = std::chrono::high_resolution_clock::now();
        } while (changed && std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() < timeout);

        _time_spent = (std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count()) * 0.000001;
        return false;
    }

    CArcIter ColoredReducer::getInArc(uint32_t pid, Colored::Transition& tran) {
        return std::find_if(tran.input_arcs.begin(), tran.input_arcs.end(), [&pid](Colored::Arc& arc) { return arc.place == pid; });
    }

    CArcIter ColoredReducer::getOutArc(Colored::Transition& tran, uint32_t pid) {
        return std::find_if(tran.output_arcs.begin(), tran.output_arcs.end(), [&pid](Colored::Arc& arc) { return arc.place == pid; });
    }
}