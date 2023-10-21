#ifndef SUPPORTRESISTANCE_H
#define SUPPORTRESISTANCE_H

#include "../../BacktestEngine.h"
#include "../../Indicator.h"
#include "../../Strategy.h"
#include "../../Data.h"
#include <vector>
class SupportResistance : public strategy::Strategy {
    std::vector<double> support;
    std::vector<double> resistance;
    std::vector<double> C;
    data::OHLC ohlc;
    
    public:
    SupportResistance(data::OHLC ohlc);
    void next() override;
};

#endif // SUPPORTRESISTANCE_H