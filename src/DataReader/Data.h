#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>
#include <stdint.h>

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

    class DataFeed {
    public:
        DataFeed() = default;

        void readCSV(const std::string& filename);
        void printOHLC() const;
        void printOHLC(uint16_t index) const;
        void printOHLC(uint16_t start, uint16_t end) const;
        void clearData();

        const OHLC& getOHLC() const;
        size_t size() const;

    private:
        OHLC ohlc;

        void parseLine(const std::string& line);
    };

}

#endif // DATA_H