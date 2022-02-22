//
// Created by nicoesterby on 22-02-2022.
//

#ifndef VERIFYPN_COLOREDREDUCER_H
#define VERIFYPN_COLOREDREDUCER_H

namespace PetriEngine {
    class ColoredPetriNetBuilder;
    namespace Colored::Reduction {

        class ColoredReducer {
        private:
            const ColoredPetriNetBuilder& _builder;
            double _time_spent = 0;
        public:
            ColoredReducer(const ColoredPetriNetBuilder& b) : _builder(b) {}

            bool reduce(uint32_t timeout);

            double time() const {
                return _time_spent;
            }
        };
    }
}

#endif //VERIFYPN_COLOREDREDUCER_H
