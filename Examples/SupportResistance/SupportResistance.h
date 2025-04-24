#ifndef SUPPORTRESISTANCE_H
#define SUPPORTRESISTANCE_H

#include "backtest/Strategy.h"
#include "backtest/Data.h"
#include <string>

class SupportResistance : public backtest::Strategy<SupportResistance, data::CSVDataFeed> {
public:
    explicit SupportResistance(data::CSVDataFeed& feed) noexcept
        : Strategy(feed) {}

    void onStart() noexcept {
        setCash(100000.0);
        setCommission(0.001);
        setSlippage(0.0);
        isLong_ = false;
        isShort_ = false;
        longQty_ = 0;
        shortQty_ = 0;
        balance_ = initialCash_;
        totalCommission_ = 0.0;
        totalSlippage_ = 0.0;
        std::cout << "========================= Starting Backtest =========================\n";
    }

    void onBar(const data::OHLCBatch& batch, std::size_t idx) noexcept {
        if (idx == 0) return;
        double prevPivot = (batch.high[idx-1] + batch.low[idx-1] + batch.close[idx-1]) / 3.0;
        double support   = 2.0 * prevPivot - batch.high[idx-1];
        double resistance= 2.0 * prevPivot - batch.low[idx-1];
        double price     = batch.open[idx];
        const std::string& date = batch.date[idx];
        if (price > prevPivot && !isLong_) {
            buy(date, price, 1);
        } else if (isLong_ && price >= resistance) {
            close(date, price, longQty_);
        } else if (price < prevPivot && !isShort_) {
            sell(date, price, 1);
        } else if (isShort_ && price <= support) {
            close(date, price, shortQty_);
        }
    }

    void onEnd() noexcept {
        std::cout << "========================= Ending Backtest =========================\n";
        stats();
    }
};

#endif // SUPPORTRESISTANCE_H
