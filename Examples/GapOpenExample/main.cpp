// GapOpenExample – standalone single-ticker demo of the GapOpenStrategy.
//
// Signal: buy at open[t] when open[t] > close[t-1] (gap up),
//         sell at close[t] (end-of-day exit).
//
// Usage:
//   GapOpenExample [csv_path] [batch_size]
//
// Default CSV: BTC-USD.csv from the shared Examples/Data directory.
//
#include <iostream>
#include <string>
#include <filesystem>

#include "backtest/Data.h"
#include "backtest/GapOpenStrategy.h"

inline std::filesystem::path dataDir() {
    std::filesystem::path p = __FILE__;
    return p.parent_path().parent_path() / "Data";
}

int main(int argc, char** argv) {
    const std::string filename =
        (argc > 1) ? argv[1] : (dataDir() / "BTC-USD.csv").string();
    const std::size_t batchSize = (argc > 2) ? std::stoul(argv[2]) : 1024;

    std::cout << "Loading: " << filename << "\n";

    data::CSVDataFeed feed(filename, batchSize);

    GapOpenStrategy strat(feed);
    strat.setCash(100000.0);
    strat.setCommission(0.001);

    strat.run();

    return 0;
}
