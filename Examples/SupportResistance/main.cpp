#include <iostream>
#include <string>
#include <cstddef>
#include <filesystem>
#include "SupportResistance.h"
#include "backtest/Data.h"

inline std::filesystem::path get_directory() {
    std::filesystem::path dir = __FILE__;
    dir = dir.parent_path().parent_path();
    return dir;
}


int main(){
    std::filesystem::path dir = get_directory();
    const std::string filename = "Data/^NSEMDCP50.csv";
    std::filesystem::path filePath = dir / filename;
    std::cout << "File Path: " << filePath << std::endl;
    const std::size_t batchSize = 1024;
    data::CSVDataFeed feed(filePath, batchSize);

    SupportResistance strat(feed);
    strat.run();
    return 0;
}
