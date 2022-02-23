//
// Created by nicoesterby on 22-02-2022.
//

#ifndef VERIFYPN_COLOREDREDUCER_H
#define VERIFYPN_COLOREDREDUCER_H

#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"
#include "PetriEngine/PQL/PlaceUseVisitor.h"
#include "ReductionRule.h"
#include "ReduceFirstPlace.h"

namespace PetriEngine::Colored {
    using CArcIter = std::vector<Arc>::iterator;

    namespace Reduction {

        class ColoredReducer {
        public:
            ColoredReducer(const PetriEngine::ColoredPetriNetBuilder& b) : _builder(b) {}

            bool reduce(uint32_t timeout, const std::vector<bool>& in_query, bool can_remove_deadlocks);

            double time() const {
                return _time_spent;
            }

            const std::vector<Colored::Place>& places() const {
                return _builder.places();
            }

            const std::vector<Colored::Transition>& transitions() const {
                return _builder.transitions();
            }

            CArcIter getInArc(uint32_t pid, Colored::Transition& tran);

            CArcIter getOutArc(Colored::Transition& tran, uint32_t pid);

        private:
            const PetriEngine::ColoredPetriNetBuilder& _builder;
            double _time_spent = 0;
            ReduceFirstPlace _reduceFirstPlace;
            std::vector<ReductionRule*> _reductions {
                // TODO Actually useful reductions. This is just a test rule to guide implementation
                &_reduceFirstPlace
            };
        };
    }
}

#endif //VERIFYPN_COLOREDREDUCER_H
