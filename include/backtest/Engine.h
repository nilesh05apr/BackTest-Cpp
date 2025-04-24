// Engine.h - high-throughput, template-driven backtest engine
#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include "backtest/Data.h"
#include <iostream>
#include <string>
#include <cstddef>

namespace backtest {

template <typename Derived, typename Feed>
class Engine {
public:
    using Batch = data::OHLCBatch;

    explicit Engine(Feed& feed) noexcept
        : feed_(feed), balance_(0.0), initialCash_(0.0), commissionRate_(0.0), totalCommission_(0.0), slippage_(0.0) {}

    void setCash(double cash) noexcept { balance_ = initialCash_ = cash; }
    void setCommission(double rate) noexcept { commissionRate_ = rate; totalCommission_ = 0.0; }
    void setSlippage(double slip) noexcept { slippage_ = slip; }

    void run() {
        static_cast<Derived*>(this)->onStart();
        while (feed_.nextBatch()) {
            const Batch& batch = feed_.currentBatch();
            for (std::size_t i = 0, n = batch.size(); i < n; ++i) {
                static_cast<Derived*>(this)->onBar(batch, i);
            }
        }
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
    void onEnd() {}
    void onBar(const Batch& batch, std::size_t idx) {}

    void buy(const std::string& date, double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * qty;
        totalCommission_ += fee;
        if (!isLong_ && !isShort_) {
            isLong_ = true; longQty_ = qty;
            balance_ -= price * qty + fee;
        } else if (isLong_) {
            longQty_ += qty;
            balance_ -= price * qty + fee;
        } else if (isShort_) {
            if (qty > shortQty_) {
                std::size_t diff = qty - shortQty_;
                isShort_ = false; isLong_ = true; longQty_ = diff; shortQty_ = 0;
                balance_ -= diff * price + fee;
            } else {
                shortQty_ -= qty;
                balance_ -= -(price * qty) + fee;
            }
        }
        std::cout << date << "\tBUY\t" << "- " << price << " \t-\t " << qty << "\n";
    }

    void sell(const std::string& date, double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * qty;
        totalCommission_ += fee;
        if (!isLong_ && !isShort_) {
            isShort_ = true; shortQty_ = qty;
            balance_ += price * qty - fee;
        } else if (isShort_) {
            shortQty_ += qty;
            balance_ += price * qty - fee;
        } else if (isLong_) {
            if (qty > longQty_) {
                std::size_t diff = qty - longQty_;
                isLong_ = false; isShort_ = true; shortQty_ = diff; longQty_ = 0;
                balance_ += diff * price - fee;
            } else {
                longQty_ -= qty;
                balance_ += price * qty - fee;
            }
        }
        std::cout << date << "\tSELL\t" << "- " << price << " \t-\t " << qty << "\n";
    }

    void close(const std::string& date, double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * qty;
        totalCommission_ += fee;
        if (isLong_) {
            balance_ += price * qty - fee;
            longQty_ -= qty;
            if (longQty_ == 0) isLong_ = false;
        } else if (isShort_) {
            balance_ -= price * qty + fee;
            shortQty_ -= qty;
            if (shortQty_ == 0) isShort_ = false;
        }
        std::cout << date << "\tCLOSE\t" << "- " << price << " \t-\t " << qty << "\n";
    }

    Feed& feed_;
    bool isLong_ = false;
    bool isShort_ = false;
    std::size_t longQty_ = 0;
    std::size_t shortQty_ = 0;
    double balance_;
    double initialCash_;
    double commissionRate_;
    double totalCommission_;
    double slippage_;
    double totalSlippage_;
};

}
#endif // BACKTEST_ENGINE_H