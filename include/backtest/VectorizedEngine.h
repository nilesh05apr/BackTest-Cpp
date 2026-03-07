// VectorizedEngine.h - vectorized backtest engine
//
// In vectorized mode the entire dataset is loaded into a single OHLCBatch
// before any strategy logic runs.  The strategy receives the complete price
// history at once and can apply whole-array operations (e.g. SMA, RSI on
// vectors) to compute entry/exit signals before iterating over the resulting
// signal arrays to record trades.
//
// Subclass VectorizedEngine<Derived, Feed> and override:
//   onStart()         – called before data is delivered
//   onData(batch)     – receives the complete dataset; compute and record trades
//   onEnd()           – called after onData returns
//
// Trade recording:
//   recordTrade(date, "BUY"|"SELL", price, qty)  – updates balance and logs
//
// Replay:
//   runFrom(startBar)   – slice the data starting at global bar index startBar
//   runFrom(startDate)  – slice the data starting at the first bar >= startDate
//
#ifndef BACKTEST_VECTORIZED_ENGINE_H
#define BACKTEST_VECTORIZED_ENGINE_H

#include "backtest/Data.h"
#include <iostream>
#include <string>
#include <cstddef>
#include <algorithm>

namespace backtest {

template <typename Derived, typename Feed>
class VectorizedEngine {
public:
    using Batch = data::OHLCBatch;

    explicit VectorizedEngine(Feed& feed) noexcept
        : feed_(feed), balance_(0.0), initialCash_(0.0),
          commissionRate_(0.0), totalCommission_(0.0), slippage_(0.0) {}

    void setCash(double cash)       noexcept { balance_ = initialCash_ = cash; }
    void setCommission(double rate) noexcept { commissionRate_ = rate; totalCommission_ = 0.0; }
    void setSlippage(double slip)   noexcept { slippage_ = slip; }

    // Run the full vectorized backtest over all available data.
    void run() {
        Batch allData = loadAll();
        static_cast<Derived*>(this)->onStart();
        static_cast<Derived*>(this)->onData(allData);
        static_cast<Derived*>(this)->onEnd();
    }

    // Run over data starting at global bar index startBar.
    void runFrom(std::size_t startBar) {
        Batch allData = loadAll();
        Batch sliced  = slice(allData, startBar, allData.size());
        static_cast<Derived*>(this)->onStart();
        static_cast<Derived*>(this)->onData(sliced);
        static_cast<Derived*>(this)->onEnd();
    }

    // Run over data starting at the first bar whose date is >= startDate.
    void runFrom(const std::string& startDate) {
        Batch allData = loadAll();
        std::size_t start = 0;
        while (start < allData.size() && allData.date[start] < startDate)
            ++start;
        Batch sliced = slice(allData, start, allData.size());
        static_cast<Derived*>(this)->onStart();
        static_cast<Derived*>(this)->onData(sliced);
        static_cast<Derived*>(this)->onEnd();
    }

    void stats() const noexcept {
        double pnl = balance_ - initialCash_;
        std::cout << "Final Balance: " << balance_ << " (P&L " << pnl << ")\n";
        std::cout << "Return: " << (pnl / initialCash_) * 100 << "%\n";
        std::cout << "Total Commission: " << totalCommission_ << "\n";
    }

protected:
    void onStart() {}
    void onData(const Batch& /*data*/) {}
    void onEnd()   {}

    // Record a single trade and update the running balance.
    // side must be "BUY" or "SELL".
    void recordTrade(const std::string& date, const std::string& side,
                     double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * static_cast<double>(qty);
        totalCommission_ += fee;
        if (side == "BUY") {
            balance_ -= price * static_cast<double>(qty) + fee;
        } else {
            balance_ += price * static_cast<double>(qty) - fee;
        }
        std::cout << date << "\t" << side << "\t" << price << "\t" << qty << "\n";
    }

    Feed&  feed_;
    double balance_;
    double initialCash_;
    double commissionRate_;
    double totalCommission_;
    double slippage_;

private:
    // Read all batches from the feed and concatenate them into one OHLCBatch.
    // Always resets the feed first so this method is safe to call more than once.
    Batch loadAll() {
        feed_.reset();
        Batch out;
        while (feed_.nextBatch()) {
            const Batch& b = feed_.currentBatch();
            out.date.insert    (out.date.end(),     b.date.begin(),     b.date.end());
            out.open.insert    (out.open.end(),     b.open.begin(),     b.open.end());
            out.high.insert    (out.high.end(),     b.high.begin(),     b.high.end());
            out.low.insert     (out.low.end(),      b.low.begin(),      b.low.end());
            out.close.insert   (out.close.end(),    b.close.begin(),    b.close.end());
            out.adjClose.insert(out.adjClose.end(), b.adjClose.begin(), b.adjClose.end());
            out.volume.insert  (out.volume.end(),   b.volume.begin(),   b.volume.end());
        }
        return out;
    }

    // Return a sub-range [from, to) of an OHLCBatch.
    static Batch slice(const Batch& src, std::size_t from, std::size_t to) {
        to = std::min(to, src.size());
        if (from >= to) return Batch{};
        Batch out;
        out.date.assign    (src.date.begin()     + static_cast<std::ptrdiff_t>(from),
                            src.date.begin()     + static_cast<std::ptrdiff_t>(to));
        out.open.assign    (src.open.begin()     + static_cast<std::ptrdiff_t>(from),
                            src.open.begin()     + static_cast<std::ptrdiff_t>(to));
        out.high.assign    (src.high.begin()     + static_cast<std::ptrdiff_t>(from),
                            src.high.begin()     + static_cast<std::ptrdiff_t>(to));
        out.low.assign     (src.low.begin()      + static_cast<std::ptrdiff_t>(from),
                            src.low.begin()      + static_cast<std::ptrdiff_t>(to));
        out.close.assign   (src.close.begin()    + static_cast<std::ptrdiff_t>(from),
                            src.close.begin()    + static_cast<std::ptrdiff_t>(to));
        out.adjClose.assign(src.adjClose.begin() + static_cast<std::ptrdiff_t>(from),
                            src.adjClose.begin() + static_cast<std::ptrdiff_t>(to));
        out.volume.assign  (src.volume.begin()   + static_cast<std::ptrdiff_t>(from),
                            src.volume.begin()   + static_cast<std::ptrdiff_t>(to));
        return out;
    }
};

} // namespace backtest
#endif // BACKTEST_VECTORIZED_ENGINE_H
