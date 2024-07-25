#ifndef SUPPORTRESISTANCE_H
#define SUPPORTRESISTANCE_H

#include "../../src/Engine/BacktestEngine.h"
#include "../../src/Indicators/Indicator.h"
#include "../../src/Strategy/Strategy.h"
#include "../../src/DataReader/Data.h"
#include <vector>
class SupportResistance : public strategy::Strategy {
    std::vector<double> support;
    std::vector<double> resistance;
    std::vector<double> C;
    data::DataFeed dataFeed;    
    public:
    SupportResistance(const data::DataFeed& dataFeed);
    void next() override;
};

#endif // SUPPORTRESISTANCE_H