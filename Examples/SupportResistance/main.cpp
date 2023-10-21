#include <iostream>
#include "SupportResistance.h"
#include "../../Data.h"


int main(){
    data::OHLC ohlc;
    data::readCSV("Data/^NSEMDCP50.csv", ohlc);
    data::printOHLC(ohlc, 0, 5);

    SupportResistance strat(ohlc);
    strat.defaultParams();
    strat.run();
    strat.stats();
    return 0;
}