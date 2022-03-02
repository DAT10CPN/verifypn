/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper van Diepen
 *      Mathias Mehl Sørensen
 */

#ifndef VERIFYPN_COLOREDPLACEUSEVISITOR_H
#define VERIFYPN_COLOREDPLACEUSEVISITOR_H

#include "PetriEngine/PQL/Visitor.h"

#include <vector>

namespace PetriEngine {
    class ColoredPetriNetBuilder;
    namespace PQL {
        class ColoredPlaceUseVisitor : public Visitor {
        public:
            ColoredPlaceUseVisitor(const ColoredPetriNetBuilder &builder);

            bool operator[](size_t id) const {
                return _in_use[id];
            }

            const std::vector<bool>& in_use() const {
                return _in_use;
            }

        protected:
            void _accept(const NotCondition *element) override;

            void _accept(const DeadlockCondition *element) override;

            void _accept(const CompareConjunction *element) override;

            void _accept(const SimpleQuantifierCondition *element) override;

            void _accept(const LogicalCondition *element) override;

            void _accept(const CompareCondition *element) override;

            void _accept(const UntilCondition *element) override;

            void _accept(const ShallowCondition *element) override;

            void _accept(const BooleanCondition *element) override;

            void _accept(const IdentifierExpr *element) override;

            void _accept(const LiteralExpr *element) override;

            void _accept(const MinusExpr *element) override;

            void _accept(const NaryExpr *element) override;

        private:
            const ColoredPetriNetBuilder &builder;
            std::vector<bool> _in_use;
        };
    }
}

#endif //VERIFYPN_COLOREDPLACEUSEVISITOR_H
