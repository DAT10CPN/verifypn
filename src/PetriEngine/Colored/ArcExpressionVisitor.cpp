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
            _singleColor = e->is_single_color();
            _colorblindWeight = e->singleton_product_weight();
        }

        void ArcExpressionVisitor::accept(const AddExpression* e) {
            _singleColor = e->is_single_color();
            _colorblindWeight = e->singleton_product_weight();
        }

        void ArcExpressionVisitor::accept(const SubtractExpression* e) {
            // todo: Maybe add support for this?
            _ok = false;
        }

        void ArcExpressionVisitor::accept(const ScalarProductExpression* e) {
            _singleColor = e->is_single_color();
            _colorblindWeight = e->singleton_product_weight();
        }
    }
}