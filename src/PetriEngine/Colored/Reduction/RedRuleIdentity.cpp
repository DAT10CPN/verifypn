/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRuleIdentity.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleIdentity::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                bool preserveDeadlocks, bool preserveConsumers, uint32_t explosion_limiter) {

        // Do nothing
        _applications = 1;

        return false;
    }
}