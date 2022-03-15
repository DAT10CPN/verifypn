//
// Created by jespo on 09-03-2022.
//

#include "PetriEngine/Colored/Expressions.h"

namespace PetriEngine {
    namespace Colored {

        uint32_t ArcExpressionVisitor::colorblindWeight(){
            return _colorblindWeight;
        }

        bool ArcExpressionVisitor::singleColor(){
            return _singleColor;
        }

        bool ArcExpressionVisitor::ok() {
            return _ok;
        }

        void ArcExpressionVisitor::reset(){
            _colorblindWeight = 1;
            _singleColor = true;
            _ok = true;
        }

        void ArcExpressionVisitor::accept(const NumberOfExpression* e) {
            _colorblindWeight += e->number();
        }

        void ArcExpressionVisitor::accept(const AddExpression* e) {
            _singleColor = false;
            for (uint32_t i = 0; i < e->size(); i++){
                e[i].visit(*this);
            }
        }

        void ArcExpressionVisitor::accept(const SubtractExpression* e) {
            _ok = false;
        }

        void ArcExpressionVisitor::accept(const ScalarProductExpression* e) {
            e->child()->visit(*this);
            _colorblindWeight *= e->scalar();
        }
    }
}