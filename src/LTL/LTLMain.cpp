/* Copyright (C) 2020  Nikolaj J. Ulrik <nikolaj@njulrik.dk>,
 *                     Simon M. Virenfeldt <simon@simwir.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LTL/LTL.h"
#include "LTL/LTLMain.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/PQL/Expressions.h"

#include "LTL/SuccessorGeneration/Spoolers.h"
#include "LTL/SuccessorGeneration/Heuristics.h"
#include "LTL/Stubborn/InterestingLTLStubbornSet.h"

#include <utility>

using namespace PetriEngine::PQL;
using namespace PetriEngine;

//#define DEBUG_EXPLORED_STATES

namespace LTL {
    struct Result {
        bool satisfied = false;
        bool is_weak = true;
        Algorithm algorithm = Algorithm::Tarjan;
#ifdef DEBUG_EXPLORED_STATES
        size_t explored_states = 0;
#endif
    };

/**
 * Converts a formula on the form A f, E f or f into just f, assuming f is an LTL formula.
 * In the case E f, not f is returned, and in this case the model checking result should be negated
 * (indicated by bool in return value)
 * @param formula - a formula on the form A f, E f or f
 * @return @code(ltl_formula, should_negate) - ltl_formula is the formula f if it is a valid LTL formula, nullptr otherwise.
 * should_negate indicates whether the returned formula is negated (in the case the parameter was E f)
 */
    std::pair<Condition_ptr, bool> to_ltl(const Condition_ptr &formula)
    {
        LTL::LTLValidator validator;
        bool should_negate = false;
        Condition_ptr converted;
        if (auto _formula = dynamic_cast<ECondition *>(formula.get())) {
            converted = std::make_shared<NotCondition>((*_formula)[0]);
            should_negate = true;
        } else if (auto _formula = dynamic_cast<ACondition *>(formula.get())) {
            converted = (*_formula)[0];
        } else {
            converted = formula;
        }
        converted->visit(validator);
        if (validator.bad()) {
            converted = nullptr;
        }
        return std::make_pair(converted, should_negate);
    }

    template<typename Checker>
    Result _verify(const PetriNet *net,
                   Condition_ptr &negatedQuery,
                   std::unique_ptr<Checker> checker,
                   const options_t &options)
    {
        Result result;
        checker->setOptions(options);
        result.satisfied = checker->isSatisfied();
        result.is_weak = checker->isweak();
#ifdef DEBUG_EXPLORED_STATES
        result.explored_states = checker->get_explored();
#endif
        if (options.printstatistics) {
            checker->printStats(std::cout);
        }
        return result;
    }

    ReturnValue LTLMain(const PetriNet *net,
                        const Condition_ptr &query,
                        const std::string &queryName,
                        options_t &options)
    {
        auto res = to_ltl(query);
        Condition_ptr negated_formula = res.first;
        bool negate_answer = res.second;

        // force AP compress off for Büchi prints
        auto compress = options.buchi_out_file.empty() ? options.ltl_compress_aps : APCompression::None;

        Structures::BuchiAutomaton automaton = makeBuchiAutomaton(negated_formula, compress);
        if (!options.buchi_out_file.empty()) {
            automaton.output_buchi(options.buchi_out_file, options.buchi_out_type);
        }

        bool is_visible_stub = options.stubbornreduction
                               && (options.ltl_por == LTLPartialOrder::Visible || options.ltl_por == LTLPartialOrder::VisibleReach)
                               && !net->has_inhibitor()
                               && !negated_formula->containsNext();
        bool is_autreach_stub = options.stubbornreduction
                && (options.ltl_por == LTLPartialOrder::AutomatonReach ||
                    options.ltl_por == LTLPartialOrder::VisibleReach)
                && !net->has_inhibitor();
        bool is_buchi_stub = options.stubbornreduction
                && options.ltl_por == LTLPartialOrder::FullAutomaton
                && !net->has_inhibitor();

        //TODO Is Torsten stub

        std::unique_ptr<SuccessorSpooler> spooler;
        std::unique_ptr<Heuristic> heuristic = nullptr;

        Result result;
        switch (options.ltlalgorithm) {
            case Algorithm::NDFS:
                if (options.strategy == PetriEngine::Reachability::RDFS) {
                    SpoolingSuccessorGenerator gen{net, negated_formula};
                    spooler = std::make_unique<EnabledSpooler>(net, gen);
                    heuristic = std::make_unique<RandomHeuristic>(options.seed());
                    gen.setSpooler(spooler.get());
                    gen.setHeuristic(heuristic.get());

                    result = _verify(net, negated_formula,
                                     std::make_unique<NestedDepthFirstSearch<SpoolingSuccessorGenerator, PetriEngine::Structures::StateSet>>(net, negated_formula, automaton, &gen),
                                     options);
                } else {
                    ResumingSuccessorGenerator gen{net};

                    if (options.trace != TraceLevel::None) {
                        result = _verify(net, negated_formula,
                                         std::make_unique<NestedDepthFirstSearch<ResumingSuccessorGenerator, PetriEngine::Structures::TracableStateSet>>(
                                                 net, negated_formula, automaton, &gen),
                                         options);
                    } else {
                        result = _verify(net, negated_formula,
                                         std::make_unique<NestedDepthFirstSearch<ResumingSuccessorGenerator, PetriEngine::Structures::StateSet>>(
                                                 net, negated_formula, automaton, &gen),
                                         options);
                    }
                }
                break;

            case Algorithm::Tarjan:
                //TODO Is Torsten stub
                if (options.strategy != PetriEngine::Reachability::DFS || is_visible_stub || is_autreach_stub) {
                    // Use spooling successor generator in case of different search strategy or stubborn set method.
                    // Running default, BestFS, or RDFS search strategy so use spooling successor generator to enable heuristics.
                    SpoolingSuccessorGenerator gen{net, negated_formula};
                    if (is_visible_stub) {
                        std::cout << "Running stubborn version!" << std::endl;
                        spooler = std::make_unique<VisibleLTLStubbornSet>(*net, negated_formula);
                    } else {
                        spooler = std::make_unique<EnabledSpooler>(net, gen);
                    }
                    std::unique_ptr<SuccessorSpooler> fallbackSpooler = nullptr;
                    if (is_autreach_stub) {
                        if (is_visible_stub) {
                            fallbackSpooler = std::make_unique<VisibleLTLStubbornSet>(*net, negated_formula);
                        } else {
                            fallbackSpooler = std::make_unique<EnabledSpooler>(net, gen);
                        }
                    }

                    if (options.strategy == PetriEngine::Reachability::RDFS) {
                        heuristic = std::make_unique<RandomHeuristic>(options.seed());
                    } else if (options.strategy == PetriEngine::Reachability::HEUR
                               || options.strategy == PetriEngine::Reachability::DEFAULT) {
                        //TODO ability to select other heuristics
                        heuristic = std::make_unique<AutomatonHeuristic>(net, automaton);
                    }
                    assert(spooler);
                    gen.setSpooler(spooler.get());
                    if (heuristic) {
                        gen.setHeuristic(heuristic.get());
                    }

                    if (options.trace != TraceLevel::None) {
                        if (is_autreach_stub && is_visible_stub) {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ReachStubProductSuccessorGenerator, SpoolingSuccessorGenerator, true, VisibleLTLStubbornSet>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen,
                                                     std::make_unique<VisibleLTLStubbornSet>(*net, negated_formula)),
                                             options);
                        }
                        else if (is_autreach_stub && !is_visible_stub) {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ReachStubProductSuccessorGenerator, SpoolingSuccessorGenerator, true, EnabledSpooler>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen,
                                                     std::make_unique<EnabledSpooler>(net, gen)),
                                             options);
                        }
                        else {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ProductSuccessorGenerator, SpoolingSuccessorGenerator, true>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen),
                                             options);
                        }
                    } else {

                        if (is_autreach_stub && is_visible_stub) {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ReachStubProductSuccessorGenerator, SpoolingSuccessorGenerator, false, VisibleLTLStubbornSet>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen,
                                                     std::make_unique<VisibleLTLStubbornSet>(*net, negated_formula)),
                                             options);
                        } else if (is_autreach_stub && !is_visible_stub) {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ReachStubProductSuccessorGenerator, SpoolingSuccessorGenerator, false, EnabledSpooler>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen,
                                                     std::make_unique<EnabledSpooler>(net, gen)),
                                             options);
                        }
                        else {
                            result = _verify(net, negated_formula,
                                             std::make_unique<TarjanModelChecker<ProductSuccessorGenerator, SpoolingSuccessorGenerator, false>>(
                                                     net,
                                                     negated_formula,
                                                     automaton,
                                                     &gen),
                                             options);
                        }
                    }
                } else {
                    ResumingSuccessorGenerator gen{net};

                    // no spooling needed, thus use resuming successor generation
                    if (options.trace != TraceLevel::None) {
                        result = _verify(net, negated_formula,
                                         std::make_unique<TarjanModelChecker<ProductSuccessorGenerator, ResumingSuccessorGenerator, true>>(
                                                 net,
                                                 negated_formula,
                                                 automaton,
                                                 &gen),
                                         options);
                    } else {
                        result = _verify(net, negated_formula,
                                         std::make_unique<TarjanModelChecker<ProductSuccessorGenerator, ResumingSuccessorGenerator, false>>(
                                                 net,
                                                 negated_formula,
                                                 automaton,
                                                 &gen),
                                         options);
                    }
                }
                break;
            case Algorithm::None:
                assert(false);
                std::cerr << "Error: cannot LTL verify with algorithm None";
        }
        std::cout << "FORMULA " << queryName
                  << (result.satisfied ^ negate_answer ? " TRUE" : " FALSE") << " TECHNIQUES EXPLICIT "
                  << LTL::to_string(options.ltlalgorithm)
                  << (result.is_weak ? " WEAK_SKIP" : "")
                  << ((options.stubbornreduction && !negated_formula->containsNext()) ? " STUBBORN" : "")
                  << std::endl;
#ifdef DEBUG_EXPLORED_STATES
        std::cout << "FORMULA " << queryName << " STATS EXPLORED " << result.explored_states << std::endl;
#endif
        return SuccessCode;
    }
}
