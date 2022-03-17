//
// Created by mathi on 09/03/2022.
//

#include <PetriEngine/Colored/PnmlWriterColorExprVisitor.h>
#include "PetriEngine/Colored/PnmlWriter.h"
#include <regex>

namespace PetriEngine {
    namespace Colored {

        void PnmlWriter::metaInfo() {
            _out << getTabs() << "<?xml version=\"1.0\"?>\n"
                 << getTabs() << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n"
                 << increaseTabs()
                 << "<net id=\"MODELID\" type=\"http://www.pnml.org/version-2009/grammar/symmetricnet\">\n"
                 << increaseTabs() << "<name><text>MODELNAME</text></name>\n";
        }

        void PnmlWriter::metaInfoClose() {
            _out << decreaseTabs() << "</net>\n"
                 << decreaseTabs() << "</pnml>";
        }

        void PnmlWriter::handleCyclicEnumeration(std::vector<const ColorType *> types) {
            _out << increaseTabs() << "<cyclicenumeration>\n";

            for (auto &type: types) {
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
                for (uint32_t i = 0; i < type->size(); i++) {
                    auto nestedType = type->operator[](i);
                    /*_out << getTabs() << "type->operator[](0) stuff: " << "\n";
                    _out << getTabs() << "ColorName: " << nestedType.getColorName() << "\n";
                    _out << getTabs() << "ToString: " << nestedType.toString() << "\n";
                    _out << getTabs() << "GetID: " << nestedType.getId() << "\n";
                    _out << getTabs() << "Actual color name " << nestedType.getActualColorName() << "\n";*/
                    _out << (first ? increaseTabs() : getTabs()) << "<feconstant id=\"" << nestedType.getColorName()
                         << "\" name= \"" << nestedType.getActualColorName() << "\"/>" << "\n";
                    first = false;
                }
            }
            _out << decreaseTabs() << "</cyclicenumeration>\n";
        }

        void PnmlWriter::handleProductSort(std::vector<const ColorType *> types) {
            _out << increaseTabs() << "<productsort>\n";
            bool first = true;
            for (auto &type: types) {
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

        void PnmlWriter::handleFiniteRange(const std::vector<const ColorType *> &types) {
            Colored::interval_t interval = types[0]->getFullInterval();
            for (auto type: types) {
                std::string start = type->operator[](0).getColorName();
                std::string end = type->operator[](type->size() - 1).getColorName();
                _out << increaseTabs() << "<finiteintrange start=\"" << start << "\" end=\"" << end << "\"/>\n";
            }
        }

        bool is_number(const std::string &s) {
            std::string::const_iterator it = s.begin();
            while (it != s.end() && std::isdigit(*it)) ++it;
            return !s.empty() && it == s.end();
        }

        void PnmlWriter::handleNamedSorts() {
            std::vector<std::string> productSorts;
            std::vector<std::string> cyclicEnumerations;
            uint32_t index = 0;
            for (auto &namedSort: _builder._colors) {
                //find all finiteint ranges first
                ColorType *colortype = const_cast<ColorType *>(namedSort.second);
                std::vector<const ColorType *> types;
                colortype->getColortypes(types);
                if (colortype->productSize() > 1) {
                    //productSorts.push_back(index);
                    productSorts.push_back(namedSort.first);
                    index++;
                    continue;
                    //this if is a hack
                } else if (is_number(types[0]->operator[](0).getColorName())) {
                    _out << getTabs() << "<namedsort id=\"" << colortype->getName() << "\" name=\""
                         << colortype->getName()
                         << "\">\n";
                    handleFiniteRange(types);
                    _out << decreaseTabs() << "</namedsort>\n";
                    continue;
                }
                cyclicEnumerations.push_back(namedSort.first);
                index++;
            }
            handleNow(productSorts, cyclicEnumerations);
        }

        void PnmlWriter::handleNow(std::vector<std::string> productSorts, std::vector<std::string> cyclicEnumerations) {


            //uint32_t index = 0;
            for (auto name: cyclicEnumerations) {
                auto *colortype = _builder._colors[name];
                //ColorType *colortype = const_cast<ColorType *>(namedSort.second);
                _out << getTabs() << "<namedsort id=\"" << colortype->getName() << "\" name=\"" << colortype->getName()
                     << "\">\n";
                std::vector<const ColorType *> types;
                colortype->getColortypes(types);
                if (types[0]->getName() == "dot") {
                    _out << increaseTabs() << "<dot/>\n";
                    _out << decreaseTabs() << "</namedsort>\n";
                    continue;
                }
                handleCyclicEnumeration(types);
                _out << decreaseTabs() << "</namedsort>\n";
            }
            for (auto name: productSorts) {
                auto *colortype = _builder._colors[name];
                //ColorType *colortype = const_cast<ColorType *>(namedSort.second);
                _out << getTabs() << "<namedsort id=\"" << colortype->getName() << "\" name=\"" << colortype->getName()
                     << "\">\n";
                std::vector<const ColorType *> types;
                colortype->getColortypes(types);
                handleProductSort(types);
                _out << decreaseTabs() << "</namedsort>\n";
            }
        }

        void PnmlWriter::handleVariables() {
            _out << getTabs() << "<!-- Declaration of user-defined color variables -->\n";
            for (auto &variable: _builder._variables) {
                _out << getTabs() << "<variabledecl id=\"" << variable.name << "\" name=\"" << variable.name << "\">\n";
                _out << increaseTabs() << "<usersort declaration=\"" << variable.colorType->getName() << "\"/>\n";
                _out << decreaseTabs() << "</variabledecl>\n";
            }
        }

        void PnmlWriter::declarations() {
            _out << getTabs() << "<!-- List of declarations -->\n";
            _out << getTabs() << "<declaration>\n";
            _out << increaseTabs() << "<structure>\n";
            _out << increaseTabs() << "<declarations>\n";
            _out << increaseTabs() << "<!-- Declaration of user-defined color classes (sorts) -->\n";

            handleNamedSorts();
            handleVariables();

            _out << decreaseTabs() << "</declarations>\n";
            _out << decreaseTabs() << "</structure>\n";
            _out << decreaseTabs() << "</declaration>\n";
        }

        std::string PnmlWriter::guardStringToPnml(std::string guard) {
            guard = std::regex_replace(guard, std::regex("<="), "lte");
            guard = std::regex_replace(guard, std::regex("<"), "lt");
            guard = std::regex_replace(guard, std::regex("=="), "eq");
            guard = std::regex_replace(guard, std::regex("!="), "neq");
            guard = std::regex_replace(guard, std::regex("&&"), "and");
            guard = std::regex_replace(guard, std::regex("\\|\\|"), "or");
            return guard;
        }

        void PnmlWriter::handleCondition(Colored::Transition &transition) {
            _out << getTabs() << "<condition>\n";
            auto guardText = guardStringToPnml(to_string(*transition.guard));
            _out << increaseTabs() << "<text>" << guardText << "</text>\n";
            _out << getTabs() << "<structure>\n";
            writeExpressionToPnml(_out, getTabsCount(), *transition.guard);
            _out << getTabs() << "</structure>\n";
            _out << decreaseTabs() << "</condition>\n";
        }

        void PnmlWriter::add_arcs_from_transition(Colored::Transition &transition) {
            for (auto &arc: transition.input_arcs) {
                if (!(std::count(_arcs.begin(), _arcs.end(), arc))) {
                    _arcs.push_back(arc);
                }
            }
            for (auto &arc: transition.output_arcs) {
                if (!(std::count(_arcs.begin(), _arcs.end(), arc))) {
                    _arcs.push_back(arc);
                }
            }
        }

        void PnmlWriter::transitions() {
            _out << getTabs() << "<!-- List of transitions -->\n";
            for (auto &transition: _builder._transitions) {
                if (transition.skipped) {
                    continue;
                }
                //add arcs to _arcs
                add_arcs_from_transition(transition);

                _out << getTabs() << "<transition id=\"" << transition.name << "\">\n";
                _out << increaseTabs() << "<name>\n";
                _out << increaseTabs() << "<text>" << transition.name
                     << "</text>\n"; //graphics location of name is not present in the struct, CBA to do it
                _out << decreaseTabs() << "</name>\n";
                _out << getTabs() << "<graphics>\n";
                _out << increaseTabs() << "<position x=\"" << transition._x << "\" y=\"" << transition._y << "\"/>\n";
                _out << decreaseTabs() << "</graphics>\n";
                if (nullptr != transition.guard) {
                    handleCondition(transition);
                }
                _out << decreaseTabs() << "</transition>\n";
            }
        }

        void PnmlWriter::handleType(const Colored::Place &place) {
            auto &type = place.type;
            _out << getTabs() << "<type>\n";
            _out << increaseTabs() << "<text>" << type->getName() << "</text>\n";
            _out << getTabs() << "<structure>\n";
            _out << increaseTabs() << "<usersort declaration=\"" << type->getName()
                 << "\"/>\n";
            _out << decreaseTabs() << "</structure>\n";
            _out << decreaseTabs() << "</type>\n";
        }

        void PnmlWriter::handlehlinitialMarking(Multiset marking) {
            _out << getTabs() << "<hlinitialMarking>\n";
            _out << increaseTabs() << "<text>" << marking.toString() << "</text>\n";
            _out << getTabs() << "<structure>\n";
            _out << increaseTabs() << "<add>\n";
            handleMarking(marking);
            _out << decreaseTabs() << "</add>\n";
            _out << decreaseTabs() << "</structure>\n";
            _out << decreaseTabs() << "</hlinitialMarking>\n";
        }

        void PnmlWriter::places() {
            _out << getTabs() << "<!-- List of places -->\n";
            for (auto &place: _builder._places) {
                if (place.skipped) {
                    continue;
                }
                _out << getTabs() << "<place id=\"" << place.name << "\">\n";
                _out << increaseTabs() << "<name>\n";
                _out << increaseTabs() << "<text>" << place.name
                     << "</text>\n"; //graphics location of name is not present in the struct, CBA to do it
                _out << decreaseTabs() << "</name>\n";
                _out << getTabs() << "<graphics>\n";
                _out << increaseTabs() << "<position x=\"" << place._x << "\" y=\"" << place._y << "\"/>\n";
                _out << decreaseTabs() << "</graphics>\n";
                handleType(place);
                if (place.marking.distinctSize() > 0) {
                    handlehlinitialMarking(place.marking);
                }
                _out << decreaseTabs() << "</place>\n";
            }
        }

        void PnmlWriter::arcs() {
            _out << getTabs() << "<!-- List of arcs -->\n";
            uint32_t index = 0;
            for (auto &arc: _arcs) {
                std::string source;
                std::string target;
                if (arc.input) {
                    source = _builder._places[arc.place].name;
                    target = _builder._transitions[arc.transition].name;
                } else {
                    source = _builder._transitions[arc.transition].name;
                    target = _builder._places[arc.place].name;
                }
                _out << getTabs() << "<arc id=\"arc" << index << "\" source=\"" << source << "\" target=\"" << target
                     << "\">\n";
                _out << increaseTabs() << "<name>\n";
                _out << increaseTabs() << "<text>" << index << "</text>\n";
                _out << decreaseTabs() << "</name>\n";
                _out << getTabs() << "<hlinscription>\n";
                _out << increaseTabs() << "<text>" << to_string(*arc.expr) << "</text>\n";
                _out << getTabs() << "<structure>\n";
                writeExpressionToPnml(_out, getTabsCount(), *arc.expr);
                _out << getTabs() << "</structure>\n";
                _out << decreaseTabs() << "</hlinscription>\n";
                _out << decreaseTabs() << "</arc>\n";
                index++;
            }
        }

        void PnmlWriter::page() {
            _out << getTabs() << "<!-- Page meta -->\n";
            _out << getTabs() << "<page id=\"page0\">\n"
                 << increaseTabs() << "<name>\n"
                 << increaseTabs() << "<text>DefaultPage</text>\n"
                 << decreaseTabs() << "</name>\n";
            places();
            transitions();
            arcs();
            _out << decreaseTabs() << "</page>\n";
        }

        void PnmlWriter::toColPNML() {
            metaInfo();
            declarations();
            page();
            metaInfoClose();
        }

        void PnmlWriter::handleMarking(Multiset marking) {
            bool first = true;
            for (auto[c, m]: marking) {
                if (first) {
                    _out << increaseTabs() << "<subterm>\n";
                } else {
                    _out << getTabs() << "<subterm>\n";
                }
                _out << increaseTabs() << "<numberof>\n";
                _out << increaseTabs() << "<subterm>" << "\n";
                _out << increaseTabs() << "<numberconstant value=\"" << m << "\">\n";
                _out << increaseTabs() << "<positive/>\n";
                _out << decreaseTabs() << "</numberconstant>\n";
                _out << decreaseTabs() << "</subterm>" << "\n";
                if (c->isTuple()) {
                    //handleTuple(c);
                    auto &colors = c->getTupleColors();
                    _out << getTabs() << "<subterm>" << "\n";
                    _out << increaseTabs() << "<tuple>\n";
                    bool firstTuple = true;
                    for (auto &color: colors) {
                        if (firstTuple) {
                            _out << increaseTabs() << "<subterm>" << "\n";
                        } else {
                            _out << getTabs() << "<subterm>" << "\n";
                        }
                        if (is_number(color->toString())) {
                            std::string start = color->getColorType()->operator[](0).getColorName();
                            std::string end = color->getColorType()->operator[](
                                    color->getColorType()->size() - 1).getColorName();
                            _out << increaseTabs() << "<finiteintrangeconstant value=\"" << color->getColorName()
                                 << "\">\n";
                            _out << increaseTabs() << "<finiteintrange end=\"" << end << "\" start=\"" << start
                                 << "\"/>\n";
                            _out << decreaseTabs() << "</finiteintrangeconstant>\n";
                        } else {
                            _out << increaseTabs() << "<usersort declaration=\"" << color->getColorName() << "\"/>\n";
                        }
                        _out << decreaseTabs() << "</subterm>" << "\n";
                        firstTuple = false;
                    }
                    _out << decreaseTabs() << "</tuple>\n";
                    _out << decreaseTabs() << "</subterm>" << "\n";
                } else {
                    _out << getTabs() << "<subterm>\n";
                    if (c->getColorName() == "Dot" || c->getColorName() == "dot") {
                        _out << increaseTabs() << "<dotconstant/>\n";
                    } else {
                        _out << increaseTabs() << "<useroperator declaration=\"" << c->getColorName() << "\"/>\n";
                    }
                    _out << decreaseTabs() << "</subterm>\n";
                }
                _out << decreaseTabs() << "</numberof>\n";
                _out << decreaseTabs() << "</subterm>\n";
                first = false;
            }
        }
    }
}
