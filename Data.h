#include <vector>
#include <string>
#include <stdint.h>

#ifndef DATA_H
#define DATA_H

namespace data {
    struct OHLC {
        std::string symbol;
        std::vector<double> open;
        std::vector<double> high;
        std::vector<double> low;
        std::vector<double> close;
        std::vector<double> adjClose;
        std::vector<double> volume;
        std::vector<std::string> date;
    };

    void readCSV(std::string filename, OHLC &ohlc);
    void printOHLC(OHLC ohlc);
    void printOHLC(OHLC ohlc, uint16_t index);
    void printOHLC(OHLC ohlc, uint16_t start, uint16_t end);
}
#endif // DATA_H
