//
// Created by mathi on 16/03/2022.
//

#include "PetriEngine/Colored/PnmlWriterColorExprVisitor.h"
#include "PetriEngine/Colored/Expressions.h"

namespace PetriEngine {
    namespace Colored {

        void writeExpressionToPnml(std::ostream &out, uint32_t tabs, const Expression &expr) {
            PnmlWriterColorExprVisitor visitor(out, tabs);
            expr.visit(visitor);
        }

        void PnmlWriterColorExprVisitor::accept(const DotConstantExpression *) {
            _out << increaseTabs() << "<dotconstant/>\n";
        }

        void PnmlWriterColorExprVisitor::accept(const VariableExpression *e) {
            _out << increaseTabs() << "<variable refvariable=\"" << e->variable()->name << "\"/>\n";
        }

        void PnmlWriterColorExprVisitor::accept(const UserOperatorExpression *e) {
            _out << increaseTabs() << "<useroperator declaration=\"" << e->user_operator()->getColorName()
                 << "\"/>\n";
        }

        void PnmlWriterColorExprVisitor::accept(const SuccessorExpression *e) {
            _out << increaseTabs() << "todo: SuccessorExpression" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const PredecessorExpression *e) {
            _out << increaseTabs() << "todo: PredecessorExpression" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const TupleExpression *tup) {
            _out << increaseTabs() << "<tuple>\n";

            for (auto& e : *tup) {
                _out << increaseTabs() << "<subterm>\n";
                e->visit(*this);
                _out << decreaseTabs() << "</subterm>\n";
            }

            _out << decreaseTabs() << "</tuple>\n";
        }

        void PnmlWriterColorExprVisitor::accept(const LessThanExpression *lt) {
            _out << increaseTabs() << "<lessthan>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*lt)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*lt)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</lessthan>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const LessThanEqExpression *lte) {
            _out << increaseTabs() << "<lessthanorequal>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*lte)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*lte)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</lessthanorequal>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const EqualityExpression *eq) {
            _out << increaseTabs() << "<equality>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*eq)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*eq)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</equality>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const InequalityExpression *neq) {
            _out << increaseTabs() << "<inequality>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*neq)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*neq)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</inequality>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AndExpression *andexpr) {
            _out << increaseTabs() << "<and>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*andexpr)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*andexpr)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</and>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const OrExpression *orexpr) {
            _out << increaseTabs() << "<or>" << "\n";
            _out << increaseTabs() << "<subterm>" << "\n";
            (*orexpr)[0]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << getTabs() << "<subterm>" << "\n";
            (*orexpr)[1]->visit(*this);
            _out << decreaseTabs() << "</subterm>" << "\n";
            _out << decreaseTabs() << "</or>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AllExpression *all) {
            _out << increaseTabs() << "<all>" << "\n";
            _out << increaseTabs() << "<usersort declaration=\"" << all->sort()->getName() << "\"/>\n";
            _out << decreaseTabs() << "</all>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const NumberOfExpression *no) {
            _out << increaseTabs() << "<numberof>" << "\n";
            if (no->is_all()) {
                _out << increaseTabs() << "<subterm>" << "\n";
                no->all()->visit(*this);
                _out << decreaseTabs() << "</subterm>" << "\n";
            } else {
                _out << increaseTabs() << "<subterm>" << "\n";
                _out << increaseTabs() << "<numberconstant value=\"" << no->number() << "\">" << "\n";
                //todo
                _out << increaseTabs() << "<positive/>\n";
                _out << decreaseTabs() << "</numberconstant>" << "\n";
                _out << decreaseTabs() << "</subterm>" << "\n";

                for (auto& c : *no) {
                    _out << increaseTabs() << "<subterm>" << "\n";
                    c->visit(*this);
                    _out << decreaseTabs() << "</subterm>" << "\n";
                }
            }
            _out << decreaseTabs() << "</numberof>" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AddExpression *add) {
            _out << increaseTabs() << "todo: AddExpression" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const SubtractExpression *sub) {
            _out << increaseTabs() << "todo: SubtractExpression" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const ScalarProductExpression *scalar) {
            _out << increaseTabs() << "todo: ScalarProductExpression" << "\n";
        }
    }
}