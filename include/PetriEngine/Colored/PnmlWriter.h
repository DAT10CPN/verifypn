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
        std::vector <Arc> _arcs;

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
                std::cout << "Underflow in tabs" << std::endl;
                return getTabs();
                //throw base_error("About to make way too many tabs, aborting");
            }
            _tabs -= 1;
            return getTabs();
        }

        void handleProductSort(std::vector<const ColorType *> types);

        void handleCyclicEnumeration(std::vector<const ColorType *> types);

        void handleFiniteRange(std::vector<const ColorType *> types);

        void handleVariables();

        void handleNamedSorts();

        void handlehlinitialMarking(Multiset marking);

        void handleType(const Place &place);

        void add_arcs_from_transition(Transition &transition);

        void handleCondition(Colored::Transition& transition);

        void handleColorExpression();
    };
}


#endif //VERIFYPN_PNMLWRITER_H
