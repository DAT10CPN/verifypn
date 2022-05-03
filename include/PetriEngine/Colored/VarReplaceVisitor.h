//
// Created by jespo on 29-04-2022.
//

#ifndef VERIFYPN_VARREPLACEVISITOR_H
#define VERIFYPN_VARREPLACEVISITOR_H

#endif //VERIFYPN_VARREPLACEVISITOR_H

#include "Expressions.h"
#include "ColorExpressionVisitor.h"

namespace PetriEngine {
    namespace Colored {
        class VarReplaceVisitor : public ColorExpressionVisitor {
        private:
            std::map<std::string, const Variable*> replacements;
            //ColorExpression copy;
            ColorExpression& expressionCache;
            GuardExpression_ptr guardCache;
        public:
            virtual void accept(const DotConstantExpression *) {
                expressionCache = DotConstantExpression();
            }

            virtual void accept(const VariableExpression *e) {
                auto swap = replacements.find(e->variable()->name);
                if (swap != replacements.end()) {
                    VariableExpression newVarEx = VariableExpression(swap->second);
                }
            }

            virtual void accept(const UserOperatorExpression *) = 0;

            virtual void accept(const SuccessorExpression *) = 0;

            virtual void accept(const PredecessorExpression *) = 0;

            virtual void accept(const TupleExpression *) = 0;

            virtual void accept(const LessThanExpression *) = 0;

            virtual void accept(const LessThanEqExpression *) = 0;

            virtual void accept(const EqualityExpression *) = 0;

            virtual void accept(const InequalityExpression *) = 0;

            virtual void accept(const AndExpression *e) {
                (*e)[0]->visit(*this);
                GuardExpression_ptr new_left = guardCache;
                (*e)[1]->visit(*this);
                GuardExpression_ptr new_right = guardCache;
                logicCache = AndExpression(new_left, new_right);
            }

            virtual void accept(const OrExpression *) = 0;

            virtual void accept(const AllExpression *) = 0;

            virtual void accept(const NumberOfExpression *) = 0;

            virtual void accept(const AddExpression *) = 0;

            virtual void accept(const SubtractExpression *) = 0;

            virtual void accept(const ScalarProductExpression *) = 0;
        };
    }
}