#include "../Engine/BacktestEngine.h"
#include "../DataReader/Data.h"
#include "../Indicators/Indicator.h"
#include <stdint.h>
#include <map>

#ifndef STRATEGY_H  
#define STRATEGY_H

namespace strategy {
    template <typename T>
    struct Params{
        std::map<std::string, T> data;

        void insert(const std::string& key, T value);
        bool get(const std::string& key, T& value) const;
        bool exists(const std::string& key) const;
        void remove(const std::string& key);
    };

    class Strategy : public backtest::BacktestEngine {
        public:
        Strategy(const data::DataFeed& dataFeed);
        void defaultParams();
        template <typename T>
        void setParams(const Params<T>& params);
        void next() override;
    };
}


#endif // STRATEGY_H