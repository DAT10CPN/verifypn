//
// Created by jespo on 09-03-2022.
//

#ifndef VERIFYPN_ARCEXPRESSIONREADVISITOR_H
#define VERIFYPN_ARCEXPRESSIONREADVISITOR_H

#endif //VERIFYPN_ARCEXPRESSIONREADVISITOR_H
#include <stdint.h>

namespace PetriEngine {
    namespace Colored {
        class NumberOfExpression;
        class AddExpression;
        class SubtractExpression;
        class ScalarProductExpression;

        class ArcExpressionVisitor {
        private:
            // This colorblindWeight is separated from the existing weight() functions because
            // the use cases disagree on the weight of a Product.
            uint32_t _colorblindWeight = 1;
            bool _singleColor = true;
            bool _ok = true;
        public:
            ArcExpressionVisitor() = default;

            uint32_t colorblindWeight();
            bool singleColor();
            bool ok();
            void reset();

            virtual void accept(const NumberOfExpression* e);
            virtual void accept(const AddExpression* e);
            virtual void accept(const SubtractExpression* e);
            virtual void accept(const ScalarProductExpression* e);
        };
    }
}