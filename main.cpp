#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include "BacktestEngine.h"
#include "Data.h"
#include "Indicator.h"
#include "Strategy.h"


int main(){
    data::OHLC ohlc;
    data::readCSV("Data/^NSEMDCP50.csv", ohlc);
    data::printOHLC(ohlc, 0, 5);


    strategy::Strategy strat(ohlc);
    strat.defaultParams();
    strat.run();
    strat.stats();
    return 0;
}