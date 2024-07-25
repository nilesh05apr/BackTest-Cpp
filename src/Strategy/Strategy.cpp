#include "Strategy.h"
#include "../Engine/BacktestEngine.h"
#include "../DataReader/Data.h"
#include "../Indicators/Indicator.h"
#include <stdint.h>
#include <map>
#include <string>
#include <variant>

namespace strategy {

    Strategy::Strategy(const data::DataFeed& dataFeed) : backtest::BacktestEngine(dataFeed) {}

    void Strategy::defaultParams() {
        this->setBalance(100000);
        this->setCommission(0.01);
        this->setSlippage(0.01);
        this->setSizer(0.01);
    }

    template <typename T>
    void Strategy::setParams(const Params<T>& params) {
        for (const auto& x : params.data) {
            if (x.first == "balance") {
                this->setBalance(std::get<double>(x.second));
            } else if (x.first == "commission") {
                this->setCommission(std::get<double>(x.second));
            } else if (x.first == "slippage") {
                this->setSlippage(std::get<double>(x.second));
            } else if (x.first == "sizer") {
                this->setSizer(std::get<double>(x.second));
            }
        }
    }

    void Strategy::next() {
        int period = 5;
        Indicators indicators(this->dataFeed);
        std::vector<double> ssma = indicators.SMA(period);
        std::vector<double> lsma = indicators.SMA(2 * period + 1);
        for (size_t i = 2 * period + 1; i < lsma.size(); i++) {
            bool longSignal = ssma[i - 1] > lsma[i - 1] && ssma[i - 2] < lsma[i - 2]; // Golden Cross
            bool shortSignal = ssma[i - 1] < lsma[i - 1] && ssma[i - 2] > lsma[i - 2]; // Death Cross
            if (longSignal) {
                this->buy(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
            } else if (shortSignal) {
                this->sell(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
            } else {
                this->close(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
            }
        }
    }

    template <typename T>
    void Params<T>::insert(const std::string& key, T value) {
        data[key] = value;
    }

    template <typename T>
    bool Params<T>::get(const std::string& key, T& value) const {
        auto it = data.find(key);
        if (it == data.end()) {
            return false;
        } else {
            value = it->second;
            return true;
        }
    }

    template <typename T>
    bool Params<T>::exists(const std::string& key) const {
        return data.find(key) != data.end();
    }

    template <typename T>
    void Params<T>::remove(const std::string& key) {
        data.erase(key);
    }

}

// Explicit template instantiation
template struct strategy::Params<double>;
template struct strategy::Params<int>;
template struct strategy::Params<std::string>;