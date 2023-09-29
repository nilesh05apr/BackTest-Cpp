#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include "BacktestEngine.h"
#include "Data.h"
#include "Indicator.h"

class Strategy : public backtest::BacktestEngine {
    private:
    data::OHLC ohlc;
    public:
    Strategy(data::OHLC ohlc);
    void next() override;
};

Strategy::Strategy(data::OHLC ohlc) : backtest::BacktestEngine(ohlc) {
    this->ohlc = ohlc;
    this->setBalance(100000);
    this->setCommission(0.01);
    this->setSlippage(0.01);
    this->setSizer(0.01);
}

void Strategy::next()  {
    int period = 5;
    Indicators indicators(this->ohlc);
    std::vector<double> sma = indicators.SMA(period);
    for(int i = period; i < sma.size(); i++){
        bool longSignal = this->ohlc.open[i] > sma[i-1]; // Gap Up
        bool shortSignal = this->ohlc.open[i] < sma[i-1]; // Gap Down
        if(longSignal){
            this->buy(this->ohlc.date[i], this->ohlc.open[i], 1);
        } else if(shortSignal){
            this->sell(this->ohlc.date[i], this->ohlc.open[i], 1);
        } else {
            this->close(this->ohlc.date[i], this->ohlc.open[i], 1);
        }
    }
}


int main(){
    data::OHLC ohlc;
    data::readCSV("Data/^NSEMDCP50.csv", ohlc);
    data::printOHLC(ohlc, 0, 5);

    Strategy strategy(ohlc);
    strategy.run();
    strategy.stats();
    return 0;
}