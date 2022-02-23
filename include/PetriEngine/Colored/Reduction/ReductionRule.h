//
// Created by nicoesterby on 23-02-2022.
//

#ifndef VERIFYPN_REDUCTIONRULE_H
#define VERIFYPN_REDUCTIONRULE_H

#include <vector>
#include <cstdint>

namespace PetriEngine::Colored::Reduction {
        class ColoredReducer;

        class ReductionRule {
        public:
            virtual bool apply(const ColoredReducer& red, const std::vector<bool>& in_query, bool can_remove_deadlocks) = 0;

            uint32_t applications() {
                return _applications;
            }

            virtual bool canBeAppliedRepeatedly() = 0;

        private:
            uint32_t _applications = 0;
        };
    }


#endif //VERIFYPN_REDUCTIONRULE_H
