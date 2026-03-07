#include <iostream>
#include <string>
#include <cstddef>
#include <filesystem>
#include "backtest/Data.h"
#include "EventDrivenSupportResistance.h"

inline std::filesystem::path get_directory() {
    std::filesystem::path dir = __FILE__;
    dir = dir.parent_path().parent_path();
    return dir;
}

// Usage:
//   EventDrivenSupportResistance [csv_path] [batch_size] [start_date|start_bar]
//
// Examples:
//   # Full run (all bars):
//   EventDrivenSupportResistance
//
//   # Replay from bar 100:
//   EventDrivenSupportResistance path/to/data.csv 1024 100
//
//   # Replay from a specific date (YYYY-MM-DD):
//   EventDrivenSupportResistance path/to/data.csv 1024 2020-01-01
//
int main(int argc, char** argv) {
    std::filesystem::path dir      = get_directory();
    const std::string     csvname  = "Data/^NSEMDCP50.csv";
    std::filesystem::path filePath = dir / csvname;

    const std::string filename  = (argc > 1 ? argv[1] : filePath.string());
    const std::size_t batchSize = (argc > 2 ? std::stoul(argv[2]) : 1024);

    data::CSVDataFeed feed(filename, batchSize);
    EventDrivenSupportResistance strat(feed);

    if (argc > 3) {
        std::string arg = argv[3];
        // Heuristic: if the argument contains '-' in ISO date position it is a
        // date string, otherwise treat it as a bar index.
        bool isDate = (arg.size() >= 10 && arg[4] == '-' && arg[7] == '-');
        if (isDate) {
            std::cout << "Replaying from date: " << arg << "\n";
            strat.runFrom(arg);
        } else {
            std::size_t startBar = std::stoul(arg);
            std::cout << "Replaying from bar: " << startBar << "\n";
            strat.runFrom(startBar);
        }
    } else {
        strat.run();
    }

    return 0;
}
