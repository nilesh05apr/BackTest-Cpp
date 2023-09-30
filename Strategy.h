#include "BacktestEngine.h"
#include "Data.h"
#include "Indicator.h"
#include <stdint.h>
#include <map>

#ifndef STRATEGY_H  
#define STRATEGY_H

namespace strategy {
    template <typename T>
    struct Params{
        std::map<std::string, T> data;

        void insert(std::string key, T value);
        bool get(std::string key, T value) const;
        bool exists(std::string key) const;
        void remove(std::string key);
    };

    class Strategy : public backtest::BacktestEngine {
        private:
        data::OHLC ohlc;
        public:
        Strategy(data::OHLC ohlc);
        void defaultParams();
        template <typename T>
        void setParams(Params<T> params);
        void next() override;
    };
}


#endif // STRATEGY_H