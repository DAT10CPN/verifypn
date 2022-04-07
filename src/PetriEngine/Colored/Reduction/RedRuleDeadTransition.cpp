/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRuleDeadTransition.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleDeadTransition::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                      QueryType queryType, bool preserveLoops, bool preserveStutter) {
        return true;
    }
}