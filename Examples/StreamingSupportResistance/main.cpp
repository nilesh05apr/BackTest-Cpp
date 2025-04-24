#include <iostream>
#include <string>
#include <cstddef>
#include <filesystem>
#include "backtest/Data.h"
#include "SupportResistance.h"


inline std::filesystem::path get_directory() {
    std::filesystem::path dir = __FILE__;
    dir = dir.parent_path().parent_path();
    return dir;
}

int main(int argc, char** argv) {
    std::filesystem::path dir = get_directory();
    const std::string csvname = "Data/^NSEMDCP50.csv";
    std::filesystem::path filePath = dir / csvname;
    const std::string filename = (argc > 1 ? argv[1] : filePath);
    const std::size_t batchSize = (argc > 2 ? std::stoul(argv[2]) : 1024);

    data::CSVDataFeed feed(filename, batchSize);
    SupportResistance strat(feed);
    strat.run();
    return 0;
}