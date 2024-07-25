#include <iostream>
#include "SupportResistance.h"
#include "../../src/DataReader/Data.h"


int main(){
    data::DataFeed dataFeed;
    dataFeed.readCSV("Data/^NSEMDCP50.csv");
    dataFeed.printOHLC(0, 5);

    SupportResistance strat(dataFeed);
    strat.defaultParams();
    strat.run();
    strat.stats();
    return 0;
}