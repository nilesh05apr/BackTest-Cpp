#ifndef VECTORIZED_SUPPORT_RESISTANCE_H
#define VECTORIZED_SUPPORT_RESISTANCE_H

#include "backtest/VectorizedEngine.h"
#include "backtest/Indicators.h"
#include "backtest/Data.h"
#include <string>
#include <cstddef>
#include <iostream>
#include <vector>

// Vectorized support/resistance strategy.
//
// The entire price history is received in one OHLCBatch.  Pivot levels are
// pre-computed for every bar as vectors, and then a single pass generates
// entry/exit signals.  Trades are recorded with recordTrade().
//
// This demonstrates using whole-array indicator computation (SMA of pivot
// levels) alongside the vectorized engine's single-pass trading loop.
//
class VectorizedSupportResistance
    : public backtest::VectorizedEngine<VectorizedSupportResistance,
                                        data::CSVDataFeed> {
public:
    explicit VectorizedSupportResistance(data::CSVDataFeed& feed) noexcept
        : VectorizedEngine(feed) {}

    void onStart() noexcept {
        setCash(100000.0);
        setCommission(0.001);
        setSlippage(0.0);
        std::cout << "========== Vectorized Backtest Starting ==========\n";
    }

    void onData(const data::OHLCBatch& batch) noexcept {
        const std::size_t n = batch.size();
        if (n < 2) return;

        // Pre-compute pivot, support, and resistance vectors for all bars.
        std::vector<double> pivot(n, 0.0);
        std::vector<double> resistance(n, 0.0);
        std::vector<double> support(n, 0.0);
        for (std::size_t i = 1; i < n; ++i) {
            pivot[i]      = (batch.high[i-1] + batch.low[i-1] + batch.close[i-1]) / 3.0;
            resistance[i] = 2.0 * pivot[i] - batch.low[i-1];
            support[i]    = 2.0 * pivot[i] - batch.high[i-1];
        }

        // Optional: smooth pivot levels with a 5-period SMA to reduce noise.
        std::vector<double> smoothPivot = backtest::indicators::SMA(pivot, 5);

        bool        isLong  = false;
        bool        isShort = false;
        std::size_t posQty  = 0; // quantity of the open position

        for (std::size_t i = 1; i < n; ++i) {
            double price = batch.open[i];
            double piv   = (smoothPivot[i] > 0.0) ? smoothPivot[i] : pivot[i];

            if (!isLong && !isShort && price > piv) {
                // Open long: size position from current balance.
                std::size_t qty = (price > 0.0)
                    ? static_cast<std::size_t>((balance_ * 0.85) / price) : 1;
                if (qty == 0) qty = 1;
                recordTrade(batch.date[i], "BUY", price, qty);
                isLong = true;
                posQty = qty;
            } else if (isLong && price >= resistance[i]) {
                // Close long: sell the exact quantity we bought.
                recordTrade(batch.date[i], "SELL", price, posQty);
                isLong = false;
                posQty = 0;
            } else if (!isLong && !isShort && price < piv) {
                // Open short: size position from current balance.
                std::size_t qty = (price > 0.0)
                    ? static_cast<std::size_t>((balance_ * 0.85) / price) : 1;
                if (qty == 0) qty = 1;
                recordTrade(batch.date[i], "SELL", price, qty);
                isShort = true;
                posQty  = qty;
            } else if (isShort && price <= support[i]) {
                // Close short: buy back the exact quantity we sold.
                recordTrade(batch.date[i], "BUY", price, posQty);
                isShort = false;
                posQty  = 0;
            }
        }
    }

    void onEnd() noexcept {
        std::cout << "========== Vectorized Backtest Ending ==========\n";
        stats();
    }
};

#endif // VECTORIZED_SUPPORT_RESISTANCE_H
