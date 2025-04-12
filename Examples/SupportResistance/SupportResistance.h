#ifndef SUPPORTRESISTANCE_H
#define SUPPORTRESISTANCE_H

#include "backtest/Strategy.h"

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
