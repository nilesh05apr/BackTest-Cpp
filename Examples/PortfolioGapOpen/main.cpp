// PortfolioGapOpen – multi-ticker portfolio demo of the GapOpenStrategy.
//
// Two tickers (BTC-USD and NSEMDCP50) are loaded from separate CSV files.
// Capital is split equally between them (equal-weight, the default).
// The GapOpenStrategy is run on each ticker:
//   - Buy at open[t] when open[t] > close[t-1] (gap up).
//   - Sell at close[t] (end-of-day exit).
//
// Per-ticker and aggregate portfolio statistics are printed.
// Trade logs are saved to the "logs/" directory.
//
#include <iostream>
#include <string>
#include <filesystem>

#include "backtest/Data.h"
#include "backtest/Portfolio.h"
#include "backtest/GapOpenStrategy.h"

inline std::filesystem::path dataDir() {
    std::filesystem::path p = __FILE__;
    return p.parent_path().parent_path() / "Data";
}

int main() {
    backtest::Portfolio portfolio("GapOpenPortfolio");

    // Two tickers – no explicit weight → equal 50 / 50 allocation.
    portfolio.addTicker("BTC-USD",   (dataDir() / "BTC-USD.csv").string());
    portfolio.addTicker("NSEMDCP50", (dataDir() / "^NSEMDCP50.csv").string());

    portfolio.setCash(200000.0);
    portfolio.setCommission(0.001);

    // Save per-ticker trade logs.
    portfolio.setTradeLogDir("logs");

    portfolio.run<GapOpenStrategy>();
    portfolio.stats();

    return 0;
}
