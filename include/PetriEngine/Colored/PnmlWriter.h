//
// Created by mathi on 09/03/2022.
//

#ifndef VERIFYPN_PNMLWRITER_H
#define VERIFYPN_PNMLWRITER_H
#include "utils/errors.h"
#include "ColoredPetriNetBuilder.h"

namespace PetriEngine::Colored {
    class PnmlWriter {
    public:
        PnmlWriter(PetriEngine::ColoredPetriNetBuilder &b, std::ostream& out) : _builder(b), _out(out), _tabs(0) {}
        void toColPNML();

    private:
        PetriEngine::ColoredPetriNetBuilder &_builder;
        std::ostream& _out;
        std::uint32_t _tabs;

        void metaInfo();

        void declarations();

        void transitions();

        void places();

        void arcs();

        void metaInfoClose();

        void page();

        std::string getTabs() {
            std::string tabsString;
            for (uint32_t i=0; i<_tabs;i++) {
                tabsString += '\t';
            }
            return tabsString;
        }

        std::string increaseTabs() {
            _tabs += 1;
            return getTabs();
        }

        std::string decreaseTabs() {
            if (_tabs == 0) {
                throw base_error("About to make way too many tabs, aborting");
            }
            _tabs -= 1;
            return getTabs();
        }

        void handleProductSort(std::vector<const ColorType *> types);

        void handleCyclicEnumeration(std::vector<const ColorType *> types);

        void handleFiniteRange(std::vector<const ColorType *> types);
    };
}


#endif //VERIFYPN_PNMLWRITER_H
