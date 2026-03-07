// GapOpenStrategy.h - Gap-up open strategy: buy when open[t] > close[t-1],
// sell at end-of-day (close[t]).
//
// Signal rule:
//   If today's open is higher than yesterday's close (a positive gap), enter
//   a long position at the open price.  The position is closed immediately at
//   today's close price, so this is always a same-bar intraday trade.
//
// Position sizing:
//   Allocate up to 95 % of current balance per trade.
//   qty = floor(balance * 0.95 / open[t])
//
// Compatible with Portfolio::run<GapOpenStrategy>() — the strategy does NOT
// call setCash() in onStart(), so the Portfolio can set the allocated cash
// before run() is invoked.
//
// Standalone usage:
//   data::CSVDataFeed feed("data.csv", 1024);
//   GapOpenStrategy strat(feed);
//   strat.setCash(100000.0);
//   strat.setCommission(0.001);
//   strat.run();
//
#ifndef BACKTEST_GAP_OPEN_STRATEGY_H
#define BACKTEST_GAP_OPEN_STRATEGY_H

#include "backtest/Strategy.h"
#include "backtest/Data.h"
#include <string>
#include <iostream>
#include <cstddef>

class GapOpenStrategy
    : public backtest::Strategy<GapOpenStrategy, data::CSVDataFeed> {
public:
    explicit GapOpenStrategy(data::CSVDataFeed& feed) noexcept
        : Strategy(feed) {}

    void onStart() noexcept {
        // Reset per-run state; respect any cash/commission set externally.
        isLong_          = false;
        isShort_         = false;
        longQty_         = 0;
        shortQty_        = 0;
        balance_         = initialCash_;
        totalCommission_ = 0.0;
        totalSlippage_   = 0.0;
        std::cout << "=== GapOpenStrategy starting (cash=" << initialCash_ << ") ===\n";
    }

    // Called for every bar in every batch.
    // idx == 0 within a batch is skipped because there is no prior bar in
    // that batch to read close[idx-1].  This is consistent with all other
    // strategy implementations in this codebase.  For feeds backed by a single
    // batch (batchSize >= number of rows, the default), only the very first bar
    // of the entire dataset is skipped.
    void onBar(const data::OHLCBatch& batch, std::size_t idx) noexcept {
        if (idx == 0) return; // no previous bar available in this batch

        const double openPrice  = batch.open[idx];
        const double closePrice = batch.close[idx];
        const double prevClose  = batch.close[idx - 1];
        const std::string& date = batch.date[idx];

        // Gap-up: today's open is above yesterday's close.
        if (openPrice > prevClose) {
            const auto qty = static_cast<std::size_t>((balance_ * 0.95) / openPrice);
            if (qty < 1) return; // not enough capital

            buy(date, openPrice, qty);   // enter at open
            close(date, closePrice, qty); // exit at EOD (close)
        }
    }

    void onEnd() noexcept {
        std::cout << "=== GapOpenStrategy finished ===\n";
        stats();
    }
};

#endif // BACKTEST_GAP_OPEN_STRATEGY_H
