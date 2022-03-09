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
