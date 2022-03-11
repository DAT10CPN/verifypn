//
// Created by mathi on 09/03/2022.
//

#include "PetriEngine/Colored/PnmlWriter.h"

namespace PetriEngine {
    namespace Colored {

        void PnmlWriter::metaInfo(){
            _out << getTabs()      << "<?xml version=\"1.0\"?>\n"
                 << getTabs() << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n"
                 << increaseTabs() << "<net id=\"MODELID\" type=\"http://www.pnml.org/version-2009/grammar/symmetricnet\">\n"
                 << getTabs() << "<name><text>MODELNAME</text></name>\n";
        }

        void PnmlWriter::metaInfoClose(){
            _out << decreaseTabs() << "</net>\n"
                 << decreaseTabs() << "</pnml>";
        }
        void PnmlWriter::handleCyclicEnumeration(std::vector<const ColorType *> types) {
            _out << increaseTabs() << "<cyclicenumeration>\n";

            for (auto &type : types) {
                if (type->productSize() > 1) {
                    throw base_error("Nested products, aborting writing PNML");
                }
                std::string name = type->getName();
                if (name == "dot") {
                    _out << increaseTabs() << "<dot/>\n";
                    break;
                }
                bool first = true;

                /*_out << getTabs() << "name of type in finite int range: " << type->getName() << "\n";
                _out << getTabs() << "type size : " << type->size() << "\n";
                _out << getTabs() << "type size : " << type->getFullInterval().toString() << "\n";*/
                for (uint32_t i=0; i<type->size(); i++) {
                    auto nestedType = type->operator[](i);
                    /*_out << getTabs() << "type->operator[](0) stuff: " << "\n";
                    _out << getTabs() << "ColorName: " << nestedType.getColorName() << "\n";
                    _out << getTabs() << "ToString: " << nestedType.toString() << "\n";
                    _out << getTabs() << "GetID: " << nestedType.getId() << "\n";
                    _out << getTabs() << "Actual color name " << nestedType.getActualColorName() << "\n";*/
                    _out << (first ? increaseTabs() : getTabs()) << "<feconstant id=\"" << nestedType.getColorName() << "\" name= \"" << nestedType.getActualColorName() << "\"/>" << "\n";
                    first = false;
                }
            }
            _out << decreaseTabs() << "</cyclicenumeration>\n";
        }

        void PnmlWriter::handleProductSort(std::vector<const ColorType *> types){
            _out << increaseTabs() << "<productsort>\n";
            bool first = true;
            for (auto &type : types) {
                std::string name = type->getName();
                if (type->productSize() > 1) {
                    throw base_error("Nested products, aborting writing PNML");
                }
                if (name == "dot") {
                    _out << increaseTabs() << "<dot/>\n";
                    break;
                }
                _out << (first ? increaseTabs() : getTabs()) << "<usersort declaration=\"" << name << "\"/>" << "\n";
                first = false;
            }
            _out << decreaseTabs() << "</productsort>\n";
        }
        void PnmlWriter::handleFiniteRange(std::vector<const ColorType *> types) {
            Colored::interval_t interval = types[0]->getFullInterval();
            for (auto type : types ) {
                std::string start = type->operator[](0).getColorName();
                std::string end = type->operator[](type->size()-1).getColorName();
                _out << increaseTabs() << "<finiteintrange start=\"" << start << "\" end=\"" << end << "\"/>\n";
            }
        }

        bool is_number(const std::string& s)
        {
            std::string::const_iterator it = s.begin();
            while (it != s.end() && std::isdigit(*it)) ++it;
            return !s.empty() && it == s.end();
        }

        void PnmlWriter::handleNamedSorts() {
            for (auto &namedSort : _builder._colors) {
                ColorType* colortype = const_cast<ColorType *>(namedSort.second);
                _out << getTabs() << "<namedsort id=\"" << colortype->getName() << "\" name=\"" << colortype->getName() << "\">\n";
                std::vector<const ColorType *> types;
                colortype->getColortypes(types);

                if (colortype->productSize() > 1) {
                    handleProductSort(types);
                    //this if is a hack
                } else if (is_number(types[0]->operator[](0).getColorName())) {
                    if (types[0]->getName() == "dot") {
                        _out << increaseTabs() << "<dot/>\n";
                        _out << decreaseTabs() << "</namedsort>\n";
                        continue;
                    }
                    handleFiniteRange(types);
                } else {
                    if (types[0]->getName() == "dot") {
                        _out << increaseTabs() << "<dot/>\n";
                        _out << decreaseTabs() << "</namedsort>\n";
                        continue;
                    }
                    handleCyclicEnumeration(types);

                }
                _out << decreaseTabs() << "</namedsort>\n";
            }
        }

        void PnmlWriter::handleVariables() {
            for (auto& variable : _builder._variables) {
                _out << getTabs() << variable.name << "\n";
            }
        }

        void PnmlWriter::declarations(){
            _out << getTabs() << "<!-- List of declarations -->\n";
            _out << increaseTabs() << "<declaration>\n";
            _out << increaseTabs() << "<structure>\n";
            _out << increaseTabs() << "<declarations>\n";
            _out << increaseTabs() << "<!-- Declaration of user-defined color classes (sorts) -->\n";

            handleNamedSorts();
            handleVariables();

            _out << decreaseTabs() << "</declarations>\n";
            _out << decreaseTabs() << "</structure>\n";
            _out << decreaseTabs() << "</declaration>\n";
        }
        void PnmlWriter::transitions(){
            _out << getTabs() << "<!-- List of transitions -->\n";
        }
        void PnmlWriter::places(){
            _out << getTabs() << "<!-- List of places -->\n";
        }
        void PnmlWriter::arcs(){
            _out << getTabs() << "<!-- List of arcs -->\n";
        }

        void PnmlWriter::page(){
            _out << getTabs() << "<!-- Page meta -->\n";
            _out << getTabs() << "<page id=\"page0\">\n"
                 << increaseTabs() << "<name>\n"
                 << increaseTabs() << "<text>DefaultPage</text>\n"
                 << decreaseTabs() << "</name>\n";
            transitions();
            places();
            arcs();
            _out << decreaseTabs() << "</page>\n";
        }

        void PnmlWriter::toColPNML() {
            metaInfo();
            declarations();
            //page();
            metaInfoClose();
        }
    }
}
