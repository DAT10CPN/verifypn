//
// Created by jespo on 09-03-2022.
//

#include "PetriEngine/Colored/Expressions.h"

// Currently has no use as its functionality was moved into the expressions. Shelved in case we end up needing to traverse ArcExpressions for a new reason.
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
            _colorblindWeight = e->weight();
        }

        void ArcExpressionVisitor::accept(const AddExpression* e) {
            _singleColor = e->is_single_color();
            _colorblindWeight = e->weight();
        }

        void ArcExpressionVisitor::accept(const SubtractExpression* e) {
            _singleColor = e->is_single_color();
            _colorblindWeight = e->weight();
        }

        void ArcExpressionVisitor::accept(const ScalarProductExpression* e) {
            _singleColor = e->is_single_color();
            _colorblindWeight = e->weight();
        }
    }
}