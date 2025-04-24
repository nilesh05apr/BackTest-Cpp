#ifndef BACKTEST_STRATEGY_H
#define BACKTEST_STRATEGY_H

#include "backtest/Engine.h"

namespace backtest {

template <typename Derived, typename Feed>
class Strategy : public Engine<Derived, Feed> {
public:
    using Engine<Derived, Feed>::Engine;
};

} // namespace backtest
#endif // BACKTEST_STRATEGY_H