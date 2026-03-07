// Portfolio.h - multi-ticker portfolio backtesting
//
// Portfolio manages a collection of securities, allocates capital across them
// (equal-weight by default), runs a strategy on each ticker, and reports
// aggregate performance statistics.
//
// Usage:
//   backtest::Portfolio portfolio("My Portfolio");
//   portfolio.addTicker("BTC-USD",    "Data/BTC-USD.csv");
//   portfolio.addTicker("NSEMDCP50",  "Data/^NSEMDCP50.csv");
//   portfolio.setCash(100000.0);
//   portfolio.setCommission(0.001);
//   portfolio.setTradeLogDir("logs");   // optional: saves per-ticker CSV logs
//   portfolio.run<MyStrategy>();
//   portfolio.stats();
//
// Weight allocation:
//   - By default each call to addTicker() uses weight=1.0, which means all
//     tickers receive equal capital.
//   - Pass an explicit weight to override: portfolio.addTicker("X", path, 1024, 0.7)
//   - Weights are normalised to sum to 1 before allocating cash.
//
#ifndef BACKTEST_PORTFOLIO_H
#define BACKTEST_PORTFOLIO_H

#include "backtest/Data.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <cstddef>

namespace backtest {

class Portfolio {
public:
    // Construct a portfolio with an optional name.
    explicit Portfolio(const std::string& name = "Portfolio")
        : name_(name), initialCash_(100000.0), commissionRate_(0.0), slippage_(0.0) {}

    // Add a ticker to the portfolio.
    // weight defaults to 1.0 (equal-weight allocation when all tickers use the
    // same default).  Weights are normalised before use, so only their relative
    // magnitudes matter.
    void addTicker(const std::string& tickerName,
                   const std::string& csvPath,
                   std::size_t        batchSize = 1024,
                   double             weight    = 1.0) {
        if (weight <= 0.0)
            throw std::invalid_argument("Ticker weight must be positive.");
        tickers_.push_back({tickerName, csvPath, batchSize, weight});
    }

    void setCash      (double cash)  noexcept { initialCash_    = cash; }
    void setCommission(double rate)  noexcept { commissionRate_ = rate; }
    void setSlippage  (double slip)  noexcept { slippage_       = slip; }

    // Directory where per-ticker trade-log CSV files are saved.
    // Files are named: <portfolio_name>_<ticker_name>_trades.csv
    // If not set, no CSV files are written.
    void setTradeLogDir(const std::string& dir) { tradeLogDir_ = dir; }

    const std::string& getName() const noexcept { return name_; }

    // Run strategy S on every ticker in the portfolio.
    // S must be constructible from (data::CSVDataFeed&) and must inherit from
    // one of the backtest engine base classes that expose:
    //   setCash(), setCommission(), setSlippage(), setTradeLogPath(), run(),
    //   getBalance(), getInitialCash(), getTotalCommission()
    template <typename S>
    void run() {
        if (tickers_.empty())
            throw std::runtime_error("Portfolio is empty – add tickers before calling run().");

        results_.clear();

        // Normalise weights.
        double totalWeight = 0.0;
        for (const auto& t : tickers_) totalWeight += t.weight;

        // Create trade-log directory if needed.
        if (!tradeLogDir_.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(tradeLogDir_, ec);
            if (ec) {
                throw std::runtime_error(
                    "Portfolio: cannot create trade-log directory '"
                    + tradeLogDir_ + "': " + ec.message());
            }
        }

        std::cout << "========== Portfolio: " << name_ << " ==========\n";

        for (const auto& t : tickers_) {
            double allocatedCash = initialCash_ * (t.weight / totalWeight);

            data::CSVDataFeed feed(t.csvPath, t.batchSize);
            S strategy(feed);

            strategy.setCash(allocatedCash);
            strategy.setCommission(commissionRate_);
            strategy.setSlippage(slippage_);

            if (!tradeLogDir_.empty()) {
                std::filesystem::path logPath =
                    std::filesystem::path(tradeLogDir_) / (name_ + "_" + t.name + "_trades.csv");
                strategy.setTradeLogPath(logPath.string());
            }

            std::cout << "---------- Ticker: " << t.name << " ----------\n";
            strategy.run();

            TickerResult res;
            res.name          = t.name;
            res.weight        = t.weight / totalWeight;
            res.initialCash   = strategy.getInitialCash();
            res.finalBalance  = strategy.getBalance();
            res.totalCommission = strategy.getTotalCommission();
            results_.push_back(res);
        }
    }

    // Print per-ticker and aggregate portfolio statistics.
    void stats() const {
        if (results_.empty()) {
            std::cout << "No results available. Call run() first.\n";
            return;
        }

        double totalInitial = 0.0;
        double totalFinal   = 0.0;
        double totalComm    = 0.0;

        std::cout << "\n========== Portfolio Stats: " << name_ << " ==========\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::left
                  << std::setw(16) << "Ticker"
                  << std::setw(10) << "Weight"
                  << std::setw(16) << "Initial Cash"
                  << std::setw(16) << "Final Balance"
                  << std::setw(12) << "P&L"
                  << std::setw(10) << "Return%"
                  << std::setw(14) << "Commission"
                  << "\n";
        std::cout << std::string(94, '-') << "\n";

        for (const auto& r : results_) {
            double pnl    = r.finalBalance - r.initialCash;
            double retPct = (r.initialCash > 0.0) ? (pnl / r.initialCash) * 100.0 : 0.0;
            std::cout << std::left
                      << std::setw(16) << r.name
                      << std::setw(10) << r.weight
                      << std::setw(16) << r.initialCash
                      << std::setw(16) << r.finalBalance
                      << std::setw(12) << pnl
                      << std::setw(10) << retPct
                      << std::setw(14) << r.totalCommission
                      << "\n";
            totalInitial += r.initialCash;
            totalFinal   += r.finalBalance;
            totalComm    += r.totalCommission;
        }

        std::cout << std::string(94, '-') << "\n";
        double totalPnl    = totalFinal - totalInitial;
        double totalRetPct = (totalInitial > 0.0) ? (totalPnl / totalInitial) * 100.0 : 0.0;
        std::cout << std::left
                  << std::setw(16) << "TOTAL"
                  << std::setw(10) << ""
                  << std::setw(16) << totalInitial
                  << std::setw(16) << totalFinal
                  << std::setw(12) << totalPnl
                  << std::setw(10) << totalRetPct
                  << std::setw(14) << totalComm
                  << "\n";
        std::cout << "=====================================================\n";
    }

private:
    struct TickerEntry {
        std::string name;
        std::string csvPath;
        std::size_t batchSize;
        double      weight;
    };

    struct TickerResult {
        std::string name;
        double      weight;
        double      initialCash;
        double      finalBalance;
        double      totalCommission;
    };

    std::string              name_;
    double                   initialCash_;
    double                   commissionRate_;
    double                   slippage_;
    std::string              tradeLogDir_;
    std::vector<TickerEntry> tickers_;
    std::vector<TickerResult> results_;
};

} // namespace backtest
#endif // BACKTEST_PORTFOLIO_H
