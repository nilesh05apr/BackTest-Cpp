#include <iostream>
#include <string>
#include <cstddef>
// Streaming example using SupportResistance strategy
#include "backtest/Data.h"
#include "SupportResistance.h"

int main(int argc, char** argv) {
    const std::string filename = (argc > 1 ? argv[1] : "../Data/^NSEMDCP50.csv");
    const std::size_t batchSize = (argc > 2 ? std::stoul(argv[2]) : 1024);

    data::CSVDataFeed feed(filename, batchSize);
    SupportResistance strat(feed);
    strat.run();
    return 0;
}