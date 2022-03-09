//
// Created by mathi on 09/03/2022.
//

#ifndef VERIFYPN_PNMLWRITER_H
#define VERIFYPN_PNMLWRITER_H

#include "ColoredPetriNetBuilder.h"

namespace PetriEngine::Colored {
    class PnmlWriter {
    public:
        PnmlWriter(PetriEngine::ColoredPetriNetBuilder &b) : _builder(b) {}
        void toColPNML(std::ostream &out);

    private:
        PetriEngine::ColoredPetriNetBuilder &_builder;
    };


}


#endif //VERIFYPN_PNMLWRITER_H
