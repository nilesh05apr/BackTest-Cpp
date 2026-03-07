#ifndef EVENT_DRIVEN_SUPPORT_RESISTANCE_H
#define EVENT_DRIVEN_SUPPORT_RESISTANCE_H

#include "backtest/EventDrivenEngine.h"
#include "backtest/Data.h"
#include <string>
#include <cstddef>
#include <iostream>

// Event-driven support/resistance strategy.
//
// Lifecycle of one bar:
//   1. Engine emits MarketEvent -> onMarket() evaluates pivot levels and
//      pushes a SignalEvent (LONG / SHORT / EXIT) when a condition is met.
//   2. onSignal() translates the signal into an OrderEvent (MARKET order).
//   3. onOrder()  fills the order at the bar's open price and pushes a
//      FillEvent.
//   4. onFill()   calls executeBuy / executeSell / executeClose to update the
//      running balance and log the trade.
//
class EventDrivenSupportResistance
    : public backtest::EventDrivenEngine<EventDrivenSupportResistance,
                                         data::CSVDataFeed> {
public:
    explicit EventDrivenSupportResistance(data::CSVDataFeed& feed) noexcept
        : EventDrivenEngine(feed) {}

    void onStart() noexcept {
        setCash(100000.0);
        setCommission(0.001);
        setSlippage(0.0);
        std::cout << "========== Event-Driven Backtest Starting ==========\n";
    }

    // ── Step 1: evaluate pivot-based conditions ───────────────────────────────
    void onMarket(const backtest::MarketEvent& evt) noexcept {
        const auto& batch = *evt.batch;
        std::size_t i     = evt.idx;
        if (i == 0) return; // need a previous bar for pivot

        double prevPivot  = (batch.high[i-1] + batch.low[i-1] + batch.close[i-1]) / 3.0;
        double resistance = 2.0 * prevPivot - batch.low[i-1];
        double support    = 2.0 * prevPivot - batch.high[i-1];
        double price      = batch.open[i];
        const std::string& date = batch.date[i];

        if (!isLong_ && !isShort_ && price > prevPivot) {
            pushSignal("ASSET", date, backtest::SignalDirection::LONG);
        } else if (isLong_ && price >= resistance) {
            pushSignal("ASSET", date, backtest::SignalDirection::EXIT);
        } else if (!isLong_ && !isShort_ && price < prevPivot) {
            pushSignal("ASSET", date, backtest::SignalDirection::SHORT);
        } else if (isShort_ && price <= support) {
            pushSignal("ASSET", date, backtest::SignalDirection::EXIT);
        }

        // Store the current price so onOrder() can use it when filling.
        pendingPrice_ = price;
        pendingDate_  = date;
    }

    // ── Step 2: translate signal into an order ────────────────────────────────
    void onSignal(const backtest::SignalEvent& evt) noexcept {
        std::size_t qty = 1;
        if (pendingPrice_ > 0.0)
            qty = static_cast<std::size_t>((balance_ * 0.85) / pendingPrice_);
        if (qty == 0) qty = 1;

        if (evt.direction == backtest::SignalDirection::LONG) {
            pushOrder(evt.symbol, evt.date,
                      backtest::OrderType::MARKET, backtest::OrderDirection::BUY, qty);
        } else if (evt.direction == backtest::SignalDirection::SHORT) {
            pushOrder(evt.symbol, evt.date,
                      backtest::OrderType::MARKET, backtest::OrderDirection::SELL, qty);
        } else { // EXIT
            std::size_t closeQty = isLong_ ? longQty_ : shortQty_;
            if (closeQty > 0)
                pushOrder(evt.symbol, evt.date,
                          backtest::OrderType::MARKET,
                          isLong_ ? backtest::OrderDirection::SELL
                                  : backtest::OrderDirection::BUY,
                          closeQty);
        }
    }

    // ── Step 3: fill the order and emit a FillEvent ───────────────────────────
    void onOrder(const backtest::OrderEvent& evt) noexcept {
        double commission = commissionRate_ * pendingPrice_ * static_cast<double>(evt.quantity);
        pushFill(evt.symbol, evt.date, evt.direction,
                 evt.quantity, pendingPrice_, commission);
    }

    // ── Step 4: apply the fill to the portfolio ───────────────────────────────
    void onFill(const backtest::FillEvent& evt) noexcept {
        if (evt.direction == backtest::OrderDirection::BUY) {
            if (isShort_) {
                // Cover short position
                executeClose(evt.date, evt.fillPrice);
            } else {
                executeBuy(evt.date, evt.fillPrice, evt.quantity);
            }
        } else {
            if (isLong_) {
                // Close long position
                executeClose(evt.date, evt.fillPrice);
            } else {
                executeSell(evt.date, evt.fillPrice, evt.quantity);
            }
        }
    }

    void onEnd() noexcept {
        std::cout << "========== Event-Driven Backtest Ending ==========\n";
        stats();
    }

private:
    double      pendingPrice_ = 0.0;
    std::string pendingDate_;
};

#endif // EVENT_DRIVEN_SUPPORT_RESISTANCE_H
