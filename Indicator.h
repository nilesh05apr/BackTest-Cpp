#include <vector>
#include "Data.h"
#include <stdint.h>

#ifndef INDICATOR_H
#define INDICATOR_H
class Indicators {
    private:
    std::vector<data::OHLC> data;
    data::OHLC ohlc;

    public:
    Indicators(std::vector<data::OHLC> data);
    Indicators(data::OHLC ohlc);
    std::vector<double> SMA(int period);
    std::vector<double> EMA(int period);
    std::vector<double> RSI(int period);
    std::vector<double> MACD(int period);
    std::vector<double> SIGNAL(int period);
    std::vector<double> ATR(int period);
    std::vector<double> BOLLINGER(int period);
    std::vector<double> STOCHASTIC(int period);
    std::vector<double> WILLIAMS(int period);
    std::vector<double> ADX(int period);
};

#endif // INDICATOR_H