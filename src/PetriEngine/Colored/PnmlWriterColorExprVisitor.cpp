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

        void PnmlWriterColorExprVisitor::accept(const DotConstantExpression*){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const VariableExpression* e){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const UserOperatorExpression* e){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const SuccessorExpression* e){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const PredecessorExpression* e){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const TupleExpression* tup){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const LessThanExpression* lt){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const LessThanEqExpression* lte){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const EqualityExpression* eq){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const InequalityExpression* neq){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AndExpression* andexpr){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const OrExpression* orexpr){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AllExpression* all){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const NumberOfExpression* no){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const AddExpression* add){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const SubtractExpression* sub){
            _out << getTabs() << "todo" << "\n";
        }

        void PnmlWriterColorExprVisitor::accept(const ScalarProductExpression* scalar){
            _out << getTabs() << "todo" << "\n";
        }
    }
}