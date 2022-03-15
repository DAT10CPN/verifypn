/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#ifndef VERIFYPN_COLOREDREDUCER_H
#define VERIFYPN_COLOREDREDUCER_H

#include <utility>

#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"
#include "PetriEngine/PQL/PlaceUseVisitor.h"
#include "ReductionRule.h"
#include "RedRulePreAgglomeration.h"


namespace PetriEngine::Colored {
    using CArcIter = __gnu_cxx::__normal_iterator<const Arc *, std::vector<Arc>>;

    namespace Reduction {

        struct ApplicationSummary {
            std::string name;
            uint32_t applications;

            ApplicationSummary(std::string name, uint32_t applications) : name(std::move(name)),
                                                                          applications(applications) {}

            bool operator<(const ApplicationSummary &rhs) const { return name < rhs.name; }
        };

        class ColoredReducer {
        public:
            ColoredReducer(PetriEngine::ColoredPetriNetBuilder &b);

            std::vector<ApplicationSummary> createApplicationSummary() const;

            bool reduce(uint32_t timeout, const std::vector<bool> &inQuery, QueryType queryType, bool preserveLoops, bool preserveStutter, uint32_t reductiontype,std::vector<uint32_t>& reductions);

            double time() const {
                return _timeSpent;
            }

            bool hasTimedOut() const {
                auto now = std::chrono::high_resolution_clock::now();
                return std::chrono::duration_cast<std::chrono::seconds>(now - _startTime).count() >= _timeout;
            }

            uint32_t placeCount() const {
                return _builder.getPlaceCount();
            }

            uint32_t transitionCount() const {
                return _builder.getTransitionCount();
            }

            uint32_t origPlaceCount() const {
                return _origPlaceCount;
            }

            uint32_t origTransitionCount() const {
                return _origTransitionCount;
            }

            uint32_t unskippedPlacesCount() const {
                return _builder.getPlaceCount() - _skippedPlaces.size();
            }

            uint32_t unskippedTransitionsCount() {
                return _builder.getTransitionCount() - _skippedTransitions.size();
            }

            const std::vector<Colored::Place> &places() const {
                return _builder.places();
            }

            const std::vector<Colored::Transition> &transitions() const {
                return _builder.transitions();
            }

            const std::vector<Colored::Arc> &inhibitorArcs() const {
                return _builder.inhibitors();
            }

            void addInputArc(const Place& place, const Transition& transition, ArcExpression_ptr& expr, uint32_t inhib_weight);
            void addOutputArc(const Transition& transition, const Place& place, ArcExpression_ptr& expr);

            CArcIter getInArc(uint32_t pid, const Colored::Transition &tran) const;
            CArcIter getOutArc(const Colored::Transition &tran, uint32_t pid) const;

            void skipPlace(uint32_t pid);

            void skipTransition(uint32_t tid);

            std::string newTransitionName();

            uint32_t newTransition(const Colored::GuardExpression_ptr& guard);

            void consistent();


        private:
            PetriEngine::ColoredPetriNetBuilder &_builder;
            std::chrono::system_clock::time_point _startTime;
            uint32_t _timeout = 0;
            double _timeSpent = 0;
            uint32_t _origPlaceCount;
            uint32_t _origTransitionCount;
            uint32_t _tnameid = 1;
            std::vector<uint32_t> _skippedPlaces;
            std::vector<uint32_t> _skippedTransitions;

            std::vector<ReductionRule *> buildApplicationSequence(std::vector<uint32_t>& reductions) {
                std::vector<ReductionRule *> specifiedReductions;
                for (auto &rule: reductions) {
                    specifiedReductions.push_back(_reductions[rule]);
                }
                return specifiedReductions;
            }

            // Reduction rules
            RedRulePreAgglomeration _preAgglomeration;
            std::vector<ReductionRule *> _reductions{
                    // TODO Actually useful reductions. This is just a test rule to guide implementation
                    &_preAgglomeration
            };
        };
    }
}

#endif //VERIFYPN_COLOREDREDUCER_H
