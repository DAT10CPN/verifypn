//
// Created by mathi on 09/03/2022.
//

#include "PetriEngine/Colored/PnmlWriter.h"
#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"

namespace PetriEngine {
    namespace Colored {

        void PnmlWriter::metaInfo(){
            _out << "<?xml version=\"1.0\"?>\n"
                 << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n"
                 << "\t<net id=\"MODELID\" type=\"http://www.pnml.org/version-2009/grammar/symmetricnet\">\n"
                 << "\t\t<name>\n"
                    "\t\t\t<text>MODELNAME</text>\n"
                    "\t\t</name>\n";
        }

        void PnmlWriter::metaInfoClose(){
            _out <<   "\t</net>\n"
                 <<   "</pnml>";
        }

        void PnmlWriter::declarations(){
            _out << "<!-- List of declarations -->\n";
            _out << "\t\t<declaration>\n";
            _out << "\t\t\t<structure>\n";
            _out << "\t\t\t\t<declarations>\n";


            for (auto &namedSort : _builder._colors) {
                ColorType* colortype = const_cast<ColorType *>(namedSort.second);
                std::string tabs = "\t\t\t\t\t";
                _out << tabs << "<namedsort id=\"" << colortype->getName() << "\" name=\"" << colortype->getName() << "\">\n";
                _out << "size: " << colortype->size() << "\n";
                _out << "product size: " << colortype->productSize() << "\n";

                std::vector<const ColorType *> types;
                colortype->getColortypes(types);

                for (auto &type : types) {
                    _out << "Name of a color in the product: " << type->getName() << "\n";
                }
                _out << tabs << "</namedsort>\n";
            }


            _out << "\t\t\t\t</declarations>\n";
            _out << "\t\t\t</structure>\n";
            _out << "\t\t</declaration>\n";
        }
        void PnmlWriter::transitions(){
            _out << "<!-- List of transitions -->\n";
        }
        void PnmlWriter::places(){
            _out << "<!-- List of places -->\n";
        }
        void PnmlWriter::arcs(){
            _out << "<!-- List of arcs -->\n";
        }

        void PnmlWriter::page(){
            _out << "<!-- Page meta -->\n";
            _out << "\t\t<page id=\"page0\">\n"
                 << "\t\t\t<name>\n"
                 << "\t\t\t\t<text>DefaultPage</text>\n"
                 << "\t\t\t</name>\n";
            transitions();
            places();
            arcs();
            _out << "\t\t</page>\n";
        }

        void PnmlWriter::toColPNML() {
            metaInfo();
            declarations();
            page();
            metaInfoClose();
        }
    }
}
