#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"
#include "PetriEngine/PQL/PlaceUseVisitor.h"

namespace PetriEngine::Colored::Reduction {

    bool ColoredReducer::reduce(uint32_t timeout) {
        if (timeout <= 0) return false;

        auto start_time = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();

        bool changed;
        do {
            changed = false;

            // TODO Reductions

            now = std::chrono::high_resolution_clock::now();
        } while (changed && std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count() >= timeout);

        _time_spent = (std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count()) * 0.000001;
        return false;
    }
}