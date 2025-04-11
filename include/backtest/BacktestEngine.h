#include "Indicator.h"
#include <vector>
#include <string>
#include <stdint.h>
#ifndef BACKTESTENGINE_H
#define BACKTESTENGINE_H

namespace backtest {
    class BacktestEngine {
        private:
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
        data::DataFeed dataFeed;
        BacktestEngine(const data::DataFeed& dataFeed);
        void setBalance(double balance);
        void setCommission(double commission);
        void setSlippage(double slippage);
        void setSizer(double sizer);
        void buy(const std::string& date, double price, uint16_t quantity);
        void sell(const std::string& date, double price, uint16_t quantity);
        void close(const std::string& date, double price, uint16_t quantity);
        void run();
        void log(const std::string& message);
        void stats();
        virtual void next() = 0;        
    };
}

#endif // BACKTESTENGINE_H
