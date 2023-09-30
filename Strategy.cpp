#include "BacktestEngine.h"
#include "Data.h"
#include "Indicator.h"
#include "Strategy.h"
#include <stdint.h>
#include <map>


namespace strategy {
    Strategy::Strategy(data::OHLC ohlc) : backtest::BacktestEngine(ohlc) {
        this->ohlc = ohlc;
    }

    void Strategy::defaultParams(){
        this->setBalance(100000);
        this->setCommission(0.01);
        this->setSlippage(0.01);
        this->setSizer(0.01);
    }

    template <typename T>
    void Strategy::setParams(Params<T> params){
        for (auto const& x : params.data){
            if(x.first == "balance"){
                this->setBalance(std::get<double>(x.second));
            } else if(x.first == "commission"){
                this->setCommission(std::get<double>(x.second));
            } else if(x.first == "slippage"){
                this->setSlippage(std::get<double>(x.second));
            } else if(x.first == "sizer"){
                this->setSizer(std::get<double>(x.second));
            }
        }
    }

    void Strategy::next()  {
        int period = 5;
        Indicators indicators(this->ohlc);
        std::vector<double> ssma = indicators.SMA(period);
        std::vector<double> lsma = indicators.SMA(2*period + 1);
        for(int i = 2*period + 1; i < lsma.size(); i++){
            bool longSignal = ssma[i-1] > lsma[i-1] && ssma[i-2] < lsma[i-2]; // Golden Cross
            bool shortSignal = ssma[i-1] < lsma[i-1] && ssma[i-2] > lsma[i-2]; // Death Cross
            if(longSignal){
                this->buy(this->ohlc.date[i], this->ohlc.open[i], 1);
            } else if(shortSignal){
                this->sell(this->ohlc.date[i], this->ohlc.open[i], 1);
            } else {
                this->close(this->ohlc.date[i], this->ohlc.open[i], 1);
            }
        }
    }

    template <typename T>
    void Params<T>::insert(std::string key, T value){
        data[key] = value;
    }

    template <typename T>
    bool Params<T>::get(std::string key, T value) const {
        auto it = data.find(key);
        if(it == data.end()){
            return false;
        } else {
            value = std::get<T>(it->second);
            return true;
        }
    }

    template <typename T>
    bool Params<T>::exists(std::string key) const {
        auto it = data.find(key);
        if(it == data.end()){
            return false;
        } else {
            return true;
        }
    }

    template <typename T>
    void Params<T>::remove(std::string key){
        auto it = data.find(key);
        if(it == data.end()){
            return;
        } else {
            data.erase(it);
        }
    }

}



