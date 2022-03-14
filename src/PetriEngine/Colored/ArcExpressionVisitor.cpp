//
// Created by jespo on 09-03-2022.
//

#include "PetriEngine/Colored/Expressions.h"

namespace PetriEngine {
    namespace Colored {

        uint32_t ArcExpressionVisitor::colorblindWeight(){
            return _colorblindWeight;
        }

        bool ArcExpressionVisitor::singleVar(){
            return _singleVar;
        }

        bool ArcExpressionVisitor::ok() {
            return _ok;
        }

        void ArcExpressionVisitor::reset(){
            _colorblindWeight = 1;
            _singleVar = true;
            _ok = true;
        }

        void ArcExpressionVisitor::accept(const NumberOfExpression* e) {
            _colorblindWeight *= e->number();
        }

        void ArcExpressionVisitor::accept(const AddExpression* e) {
            _singleVar = false;
            for (uint32_t i = 0; i < e->size(); i++){
                e[i].visit(*this);
            }
        }

        void ArcExpressionVisitor::accept(const SubtractExpression* e) {
            _ok = false;
        }

        void ArcExpressionVisitor::accept(const ScalarProductExpression* e) {
            _colorblindWeight *= e->scalar();
            e->child()->visit(*this);
        }
    }
}