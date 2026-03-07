// PortfolioExample – demonstrates multi-ticker portfolio backtesting.
//
// Two tickers are added to a Portfolio with default equal weights.
// The same SupportResistance strategy is run on each ticker.
// Per-ticker and aggregate statistics are printed, and trade logs are saved
// to CSV files under the "logs/" directory.
//
#include <iostream>
#include <string>
#include <filesystem>

#include "backtest/Data.h"
#include "backtest/Strategy.h"
#include "backtest/Portfolio.h"

// ── Inline SupportResistance strategy ────────────────────────────────────────

class SupportResistance
    : public backtest::Strategy<SupportResistance, data::CSVDataFeed> {
public:
    explicit SupportResistance(data::CSVDataFeed& feed) noexcept
        : Strategy(feed) {}

    void onStart() noexcept {
        isLong_  = false;
        isShort_ = false;
        longQty_  = 0;
        shortQty_ = 0;
        balance_         = initialCash_;
        totalCommission_ = 0.0;
        totalSlippage_   = 0.0;
        std::cout << "  -- Starting strategy (cash=" << initialCash_ << ")\n";
    }

    void onBar(const data::OHLCBatch& batch, std::size_t idx) noexcept {
        if (idx == 0) return;
        double prevPivot  = (batch.high[idx-1] + batch.low[idx-1] + batch.close[idx-1]) / 3.0;
        double support    = 2.0 * prevPivot - batch.high[idx-1];
        double resistance = 2.0 * prevPivot - batch.low[idx-1];
        double price      = batch.open[idx];
        const std::string& date = batch.date[idx];

        int qty = static_cast<int>((balance_ * 0.85) / price);
        if (qty < 1) return;

        if (price > prevPivot && !isLong_) {
            buy(date, price, static_cast<std::size_t>(qty));
        } else if (isLong_ && price >= resistance) {
            close(date, price, longQty_);
        } else if (price < prevPivot && !isShort_) {
            sell(date, price, static_cast<std::size_t>(qty));
        } else if (isShort_ && price <= support) {
            close(date, price, shortQty_);
        }
    }

    void onEnd() noexcept {
        std::cout << "  -- Strategy finished\n";
    }
};

// ── main ─────────────────────────────────────────────────────────────────────

inline std::filesystem::path dataDir() {
    std::filesystem::path p = __FILE__;
    return p.parent_path().parent_path() / "Data";
}

int main() {
    backtest::Portfolio portfolio("DemoPortfolio");

    // Add two tickers – no explicit weights means equal allocation (50 / 50).
    portfolio.addTicker("BTC-USD",   (dataDir() / "BTC-USD.csv").string());
    portfolio.addTicker("NSEMDCP50", (dataDir() / "^NSEMDCP50.csv").string());

    portfolio.setCash(200000.0);
    portfolio.setCommission(0.001);

    // Save per-ticker trade logs to the "logs" directory.
    portfolio.setTradeLogDir("logs");

    portfolio.run<SupportResistance>();
    portfolio.stats();

    return 0;
}
