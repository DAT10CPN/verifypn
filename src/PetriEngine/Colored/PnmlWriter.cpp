//
// Created by mathi on 09/03/2022.
//

#include "PetriEngine/Colored/PnmlWriter.h"
#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"

namespace PetriEngine {
    namespace Colored {

        void PnmlWriter::toColPNML(std::ostream &out) {
            out << "<?xml version=\"1.0\"?>\n"
            << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n"
            << "<net id=\"veryniceID\" type=\"http://www.pnml.org/version-2009/grammar/symmetricnet\">\n";
            for (auto &it: _builder._colors) {
                out << it.first;
            }
            /*out << "<name><text>veryniceID</text></name>\n";

            out << "<declaration>\n";
            out << "<structure>\n";
            out << "<declarations>\n";
            //Go through all structures
            for (auto &it: _colors) {
            out << it.first;
            }


            out << "</declarations>\n";
            out << "</structure>\n";
            out << "</declaration>\n";

            //Go through all transition/places
            out << "<name><text>DefaultPage</text></name>";

            //Places
            for (auto &_place: _places) {
            //auto& p = _placenames[_place.name];
            //auto& placelocation = _placelocations[i];
            out << "<place id=\"" << _place.name << "\">\n";
            << "<graphics><position x=\"" << std::get<0>(placelocation)
            << "\" y=\"" << std::get<1>(placelocation) << "\"/></graphics>\n"
            //<< "<name><text>" << p << "</text></name>\n";
            if(_initialMarking[i] > 0)
            {
            out << "<initialMarking><text>" << _initialMarking[i] << "</text></initialMarking>\n";
            }
            out << "</place>\n";
            }

            //Transitions


            out << "</net>\n"
            << "</pnml>";*/
        }
    }
}
