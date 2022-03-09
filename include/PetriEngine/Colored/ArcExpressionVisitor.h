//
// Created by jespo on 09-03-2022.
//

#ifndef VERIFYPN_ARCEXPRESSIONREADVISITOR_H
#define VERIFYPN_ARCEXPRESSIONREADVISITOR_H

#endif //VERIFYPN_ARCEXPRESSIONREADVISITOR_H

#include "Expressions.h"

namespace PetriEngine {
        namespace Colored {
            class ArcExpressionVisitor {
            private:
                uint32_t k = 1;
                bool singleVariable = true;
                bool ok = true;
            public:
                ArcExpressionVisitor() = default;

                virtual void accept(const NumberOfExpression& e){
                    k *= e.number();
                }
                virtual void accept(const AddExpression& e){
                    singleVariable = false;
                }
                virtual void accept(const SubtractExpression& e){
                    ok = false;
                }
                virtual void accept(const ScalarProductExpression& e){
                    k *= e.scalar();
                    accept(e.child());
                }
            }
        }
}