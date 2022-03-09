//
// Created by mathi on 09/03/2022.
//

#ifndef VERIFYPN_PNMLWRITER_H
#define VERIFYPN_PNMLWRITER_H

#include "ColoredPetriNetBuilder.h"

namespace PetriEngine::Colored {
    class PnmlWriter {
    public:
        PnmlWriter(PetriEngine::ColoredPetriNetBuilder &b, std::ostream& out) : _builder(b), _out(out) {}
        void toColPNML();

    private:
        PetriEngine::ColoredPetriNetBuilder &_builder;
        std::ostream& _out;

        void metaInfo();

        void declarations();

        void transitions();

        void places();

        void arcs();

        void metaInfoClose();

        void page();
    };


}


#endif //VERIFYPN_PNMLWRITER_H
