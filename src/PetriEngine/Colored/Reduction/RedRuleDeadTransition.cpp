/*
 * Authors:
 *      Nicolaj Østerby Jensen
 *      Jesper Adriaan van Diepen
 *      Mathias Mehl Sørensen
 */

#include "PetriEngine/Colored/Reduction/RedRuleDeadTransition.h"
#include "PetriEngine/Colored/Reduction/ColoredReducer.h"

namespace PetriEngine::Colored::Reduction {
    bool RedRuleDeadTransition::apply(ColoredReducer &red, const std::vector<bool> &inQuery,
                                      QueryType queryType, bool preserveLoops, bool preserveStutter) {

        bool continueReductions = false;
        const size_t numberofplaces = red.placeCount();
        for(uint32_t p = 0; p < numberofplaces; ++p)
        {
            if(red.hasTimedOut()) return false;
            Place place = red.places()[p];
            if(place.skipped) continue;
            if(place.inhibitor) continue;
            if(place._pre.size() > place._post.size()) continue;

            std::set<uint32_t> notenabled;
            bool ok = true;
            for(uint cons : place._post)
            {
                Transition t = red.transitions()[cons];
                auto in = red.getInArc(p, t);
                if(in->expr->weight() <= place.marking.size())
                {
                    auto out = red.getOutArc(t, p);
                    if(out == t.output_arcs.end() || out->place != p || out->expr->weight() >= in->expr->weight())
                    {
                        ok = false;
                        break;
                    }
                }
                else
                {
                    notenabled.insert(cons);
                }
            }

            if(!ok || notenabled.size() == 0) continue;

            for(uint prod : place._pre)
            {
                if(notenabled.count(prod) == 0)
                {
                    ok = false;
                    break;
                }
                // check that producing arcs originate from transition also
                // consuming. If so, we know it will never fire.
                Transition t = red.transitions()[prod];
                CArcIter it = red.getInArc(p, t);
                if(it == t.input_arcs.end())
                {
                    ok = false;
                    break;
                }
            }

            if(!ok) continue;

            _applications++;
            continueReductions = true;

            if(inQuery[p] == 0)
                place.marking.size() == 0;

            bool skipplace = (notenabled.size() == place._post.size()) && (inQuery[p] == 0);
            for(uint cons : notenabled)
                red.skipTransition(cons);

            if(skipplace)
                red.skipPlace(p);

        }
        red.consistent();
        return continueReductions;
    }
}