// EventDrivenEngine.h - event-driven backtest engine
//
// In event-driven mode the engine maintains an event queue.  Each new bar
// produces a MarketEvent.  The strategy processes that event and may push
// SignalEvents into the queue.  Each SignalEvent is translated into an
// OrderEvent, which is filled immediately to produce a FillEvent.  The
// FillEvent updates the portfolio state.
//
// Subclass EventDrivenEngine<Derived, Feed> and override:
//   onStart()             – called before the first bar
//   onMarket(evt)         – react to a new bar; push signal(s) if desired
//   onSignal(evt)         – translate a signal into an order; push order
//   onOrder(evt)          – fill the order; push fill
//   onFill(evt)           – update position / log the fill
//   onEnd()               – called after the last bar
//
// Helper methods:
//   pushSignal(...)       – enqueue a SignalEvent
//   pushOrder(...)        – enqueue an OrderEvent
//   pushFill(...)         – enqueue a FillEvent
//   executeBuy/Sell/Close – update balance and log the trade
//   runFrom(startBar)     – replay from a global bar index
//   runFrom(startDate)    – replay from the first bar with date >= startDate
//
#ifndef BACKTEST_EVENT_DRIVEN_ENGINE_H
#define BACKTEST_EVENT_DRIVEN_ENGINE_H

#include "backtest/Data.h"
#include <queue>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <cstddef>

namespace backtest {

// ── Event types ──────────────────────────────────────────────────────────────

enum class EventType { MARKET, SIGNAL, ORDER, FILL };

struct Event {
    EventType type;
    virtual ~Event() = default;
protected:
    explicit Event(EventType t) : type(t) {}
};

// Carries a pointer to the current batch and the bar index within it.
struct MarketEvent : Event {
    const data::OHLCBatch* batch = nullptr;
    std::size_t idx = 0;
    MarketEvent(const data::OHLCBatch* b, std::size_t i)
        : Event(EventType::MARKET), batch(b), idx(i) {}
};

enum class SignalDirection { LONG, SHORT, EXIT };

struct SignalEvent : Event {
    std::string symbol;
    std::string date;
    SignalDirection direction;
    double strength = 1.0;
    SignalEvent(std::string sym, std::string d, SignalDirection dir, double s = 1.0)
        : Event(EventType::SIGNAL), symbol(std::move(sym)), date(std::move(d)),
          direction(dir), strength(s) {}
};

enum class OrderDirection { BUY, SELL };
enum class OrderType     { MARKET, LIMIT };

struct OrderEvent : Event {
    std::string symbol;
    std::string date;
    OrderType     orderType;
    OrderDirection direction;
    std::size_t   quantity;
    double        limitPrice = 0.0;
    OrderEvent(std::string sym, std::string d, OrderType ot,
               OrderDirection dir, std::size_t qty, double lp = 0.0)
        : Event(EventType::ORDER), symbol(std::move(sym)), date(std::move(d)),
          orderType(ot), direction(dir), quantity(qty), limitPrice(lp) {}
};

struct FillEvent : Event {
    std::string    symbol;
    std::string    date;
    OrderDirection direction;
    std::size_t    quantity;
    double         fillPrice;
    double         commission;
    FillEvent(std::string sym, std::string d, OrderDirection dir,
              std::size_t qty, double fp, double comm)
        : Event(EventType::FILL), symbol(std::move(sym)), date(std::move(d)),
          direction(dir), quantity(qty), fillPrice(fp), commission(comm) {}
};

using EventQueue = std::queue<std::unique_ptr<Event>>;

// ── EventDrivenEngine ─────────────────────────────────────────────────────────

template <typename Derived, typename Feed>
class EventDrivenEngine {
public:
    using Batch = data::OHLCBatch;

    explicit EventDrivenEngine(Feed& feed) noexcept
        : feed_(feed), balance_(0.0), initialCash_(0.0),
          commissionRate_(0.0), totalCommission_(0.0), slippage_(0.0) {}

    void setCash(double cash)       noexcept { balance_ = initialCash_ = cash; }
    void setCommission(double rate) noexcept { commissionRate_ = rate; totalCommission_ = 0.0; }
    void setSlippage(double slip)   noexcept { slippage_ = slip; }

    // Open a CSV file and write all subsequent trades to it.
    // The file is created with the header: date,action,price,qty
    // Each row is flushed immediately for durability.
    void setTradeLogPath(const std::string& path) {
        tradeLog_.open(path);
        if (tradeLog_.is_open()) {
            tradeLog_ << "date,action,price,qty\n";
            tradeLog_.flush();
        }
    }

    double getBalance()         const noexcept { return balance_; }
    double getInitialCash()     const noexcept { return initialCash_; }
    double getTotalCommission() const noexcept { return totalCommission_; }

    // Run the full backtest.
    void run() {
        static_cast<Derived*>(this)->onStart();
        while (feed_.nextBatch()) {
            const Batch& batch = feed_.currentBatch();
            for (std::size_t i = 0, n = batch.size(); i < n; ++i) {
                eventQueue_.push(std::make_unique<MarketEvent>(&batch, i));
                drainQueue();
            }
        }
        static_cast<Derived*>(this)->onEnd();
    }

    // Replay from a specific global bar index (0-based).
    void runFrom(std::size_t startBar) {
        feed_.reset();
        static_cast<Derived*>(this)->onStart();
        std::size_t globalBar = 0;
        while (feed_.nextBatch()) {
            const Batch& batch = feed_.currentBatch();
            for (std::size_t i = 0, n = batch.size(); i < n; ++i, ++globalBar) {
                if (globalBar < startBar) continue;
                eventQueue_.push(std::make_unique<MarketEvent>(&batch, i));
                drainQueue();
            }
        }
        static_cast<Derived*>(this)->onEnd();
    }

    // Replay from the first bar whose date string is >= startDate.
    void runFrom(const std::string& startDate) {
        feed_.reset();
        static_cast<Derived*>(this)->onStart();
        bool active = false;
        while (feed_.nextBatch()) {
            const Batch& batch = feed_.currentBatch();
            for (std::size_t i = 0, n = batch.size(); i < n; ++i) {
                if (!active && batch.date[i] < startDate) continue;
                active = true;
                eventQueue_.push(std::make_unique<MarketEvent>(&batch, i));
                drainQueue();
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

    // ── Convenience helpers to push events from strategy overrides ────────────

    void pushSignal(std::string symbol, std::string date,
                    SignalDirection dir, double strength = 1.0) {
        eventQueue_.push(std::make_unique<SignalEvent>(
            std::move(symbol), std::move(date), dir, strength));
    }

    void pushOrder(std::string symbol, std::string date,
                   OrderType ot, OrderDirection dir,
                   std::size_t qty, double lp = 0.0) {
        eventQueue_.push(std::make_unique<OrderEvent>(
            std::move(symbol), std::move(date), ot, dir, qty, lp));
    }

    void pushFill(std::string symbol, std::string date,
                  OrderDirection dir, std::size_t qty,
                  double fillPrice, double commission) {
        eventQueue_.push(std::make_unique<FillEvent>(
            std::move(symbol), std::move(date), dir, qty, fillPrice, commission));
    }

protected:
    // Override these in your strategy.
    void onStart() {}
    void onEnd()   {}
    void onMarket(const MarketEvent& /*evt*/) {}
    void onSignal(const SignalEvent& /*evt*/) {}
    void onOrder (const OrderEvent&  /*evt*/) {}
    void onFill  (const FillEvent&   /*evt*/) {}

    // ── Trade execution helpers ───────────────────────────────────────────────

    void executeBuy(const std::string& date, double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * static_cast<double>(qty);
        totalCommission_ += fee;
        balance_ -= price * static_cast<double>(qty) + fee;
        isLong_ = true;
        longQty_ += qty;
        std::cout << date << "\tBUY\t" << price << "\t" << qty << "\n";
        if (tradeLog_.is_open()) { tradeLog_ << date << ",BUY," << price << "," << qty << "\n"; tradeLog_.flush(); }
    }

    void executeSell(const std::string& date, double price, std::size_t qty) noexcept {
        double fee = commissionRate_ * price * static_cast<double>(qty);
        totalCommission_ += fee;
        balance_ += price * static_cast<double>(qty) - fee;
        isShort_ = true;
        shortQty_ += qty;
        std::cout << date << "\tSELL\t" << price << "\t" << qty << "\n";
        if (tradeLog_.is_open()) { tradeLog_ << date << ",SELL," << price << "," << qty << "\n"; tradeLog_.flush(); }
    }

    void executeClose(const std::string& date, double price) noexcept {
        if (isLong_) {
            double fee = commissionRate_ * price * static_cast<double>(longQty_);
            totalCommission_ += fee;
            balance_ += price * static_cast<double>(longQty_) - fee;
            std::cout << date << "\tCLOSE LONG\t" << price << "\t" << longQty_ << "\n";
            if (tradeLog_.is_open()) { tradeLog_ << date << ",CLOSE LONG," << price << "," << longQty_ << "\n"; tradeLog_.flush(); }
            longQty_ = 0;
            isLong_  = false;
        } else if (isShort_) {
            double fee = commissionRate_ * price * static_cast<double>(shortQty_);
            totalCommission_ += fee;
            balance_ -= price * static_cast<double>(shortQty_) + fee;
            std::cout << date << "\tCLOSE SHORT\t" << price << "\t" << shortQty_ << "\n";
            if (tradeLog_.is_open()) { tradeLog_ << date << ",CLOSE SHORT," << price << "," << shortQty_ << "\n"; tradeLog_.flush(); }
            shortQty_ = 0;
            isShort_ = false;
        }
    }

    Feed&      feed_;
    EventQueue eventQueue_;
    bool       isLong_  = false;
    bool       isShort_ = false;
    std::size_t longQty_  = 0;
    std::size_t shortQty_ = 0;
    double balance_;
    double initialCash_;
    double commissionRate_;
    double totalCommission_;
    double slippage_;

private:
    std::ofstream tradeLog_;

    void drainQueue() {
        while (!eventQueue_.empty()) {
            auto& evtPtr = eventQueue_.front();
            switch (evtPtr->type) {
                case EventType::MARKET:
                    static_cast<Derived*>(this)->onMarket(
                        static_cast<const MarketEvent&>(*evtPtr));
                    break;
                case EventType::SIGNAL:
                    static_cast<Derived*>(this)->onSignal(
                        static_cast<const SignalEvent&>(*evtPtr));
                    break;
                case EventType::ORDER:
                    static_cast<Derived*>(this)->onOrder(
                        static_cast<const OrderEvent&>(*evtPtr));
                    break;
                case EventType::FILL:
                    static_cast<Derived*>(this)->onFill(
                        static_cast<const FillEvent&>(*evtPtr));
                    break;
            }
            eventQueue_.pop();
        }
    }
};

} // namespace backtest
#endif // BACKTEST_EVENT_DRIVEN_ENGINE_H
