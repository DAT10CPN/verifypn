//
// Created by jespo on 09-03-2022.
//

#ifndef VERIFYPN_ARCEXPRESSIONREADVISITOR_H
#define VERIFYPN_ARCEXPRESSIONREADVISITOR_H

#endif //VERIFYPN_ARCEXPRESSIONREADVISITOR_H

#include "Expressions.h"

namespace PetriEngine {
    namespace Colored {
        class NumberOfExpression;
        class AddExpression;
        class SubtractExpression;
        class ScalarProductExpression;

        class ArcExpressionVisitor {
        private:
            uint32_t k = 1;
            bool singleVariable = true;
            bool ok = true;
        public:
            ArcExpressionVisitor() = default;

            virtual void accept(const NumberOfExpression& e);
            virtual void accept(const AddExpression& e);
            virtual void accept(const SubtractExpression& e);
            virtual void accept(const ScalarProductExpression& e);
        };
    }
}