#include "Data.h"
#include "Indicator.h"
#include <vector>
#include <string>
#include <stdint.h>
#ifndef BACKTESTENGINE_H
#define BACKTESTENGINE_H

namespace backtest {
    class BacktestEngine {
        private:
        std::vector<data::OHLC> data;
        data::OHLC ohlc;
        bool isLong;
        bool isShort;
        bool Position;
        uint16_t quantity;
        uint16_t longQuantity;
        uint16_t shortQuantity;
        double balance;
        double initialBalance;
        double finalBalance;
        double commission;
        double totalCommission;
        double slippage;
        double totalSlippage;
        double sizer;
        double risk;




        public:
        BacktestEngine(std::vector<data::OHLC> data);
        BacktestEngine(data::OHLC ohlc);
        void setBalance(double balance);
        void setCommission(double commission);
        void setSlippage(double slippage);
        void setSizer(double sizer);
        void buy(std::string date, double price, uint16_t quantity);
        void sell(std::string date, double price, uint16_t quantity);
        void close(std::string date, double price, uint16_t quantity);
        void run();
        void log(std::string message);
        void stats();
        virtual void next() = 0;        
    };
}

#endif // BACKTESTENGINE_H