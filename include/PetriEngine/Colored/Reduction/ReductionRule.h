/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper van Diepen
 *      Mathias Mehl Sørensen
 */

#ifndef VERIFYPN_REDUCTIONRULE_H
#define VERIFYPN_REDUCTIONRULE_H

#include <vector>
#include <cstdint>
#include <string>

namespace PetriEngine::Colored::Reduction {
        class ColoredReducer;

        class ReductionRule {
        public:
            virtual std::string name() = 0;
            virtual bool canBeAppliedRepeatedly() = 0;

            uint32_t applications() {
                return _applications;
            }

            virtual bool apply(ColoredReducer& red, const std::vector<bool>& in_query, bool can_remove_deadlocks) = 0;

        protected:
            uint32_t _applications = 0;
        };
    }


#endif //VERIFYPN_REDUCTIONRULE_H
