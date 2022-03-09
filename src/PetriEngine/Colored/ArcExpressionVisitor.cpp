//
// Created by jespo on 09-03-2022.
//

#include "PetriEngine/Colored/ArcExpressionVisitor.h"
#include "PetriEngine/Colored/Expressions.h"

namespace PetriEngine {
    namespace Colored {
        void ArcExpressionVisitor::accept(const NumberOfExpression &e) {
            k *= e.number();
        }

        void ArcExpressionVisitor::accept(const AddExpression &e) {
            singleVariable = false;
        }

        void ArcExpressionVisitor::accept(const SubtractExpression &e) {
            ok = false;
        }

        void ArcExpressionVisitor::accept(const ScalarProductExpression &e) {
            k *= e.scalar();
            e.child()->visit(*this);
        }
    }
}