#include "backtest/Data.h"
#include "backtest/BacktestEngine.h"
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

namespace backtest {

    BacktestEngine::BacktestEngine(const data::DataFeed& dataFeed) : dataFeed(dataFeed), isLong(false), isShort(false), Position(false),
        quantity(0), longQuantity(0), shortQuantity(0), balance(0), commission(0), slippage(0), sizer(0),
        totalCommission(0), totalSlippage(0), initialBalance(0), finalBalance(0) {}

    void BacktestEngine::setBalance(double balance) {
        this->balance = balance;
    }

    void BacktestEngine::setCommission(double commission) {
        this->commission = commission;
        this->totalCommission = 0.0;
    }

    void BacktestEngine::setSlippage(double slippage) {
        this->slippage = slippage;
    }

    void BacktestEngine::setSizer(double sizer) {
        this->sizer = sizer;
    }

    void BacktestEngine::buy(const std::string& date, double price, uint16_t quantity) {
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;

        if (!this->Position) {
            this->isLong = true;
            this->Position = true;
            this->longQuantity = quantity;
            this->balance -= (price * quantity) + _comm;
        } else if (this->isLong) {
            this->longQuantity += quantity;
            this->balance -= (price * quantity) + _comm;
        } else if (this->isShort && quantity > this->shortQuantity) {
            this->isShort = false;
            this->isLong = true;
            this->longQuantity = quantity - this->shortQuantity;
            this->shortQuantity = 0;
            this->balance -= (price * quantity) + _comm;
        } else if (this->isShort && quantity <= this->shortQuantity) {
            this->shortQuantity -= quantity;
            this->balance += (price * quantity) - _comm;
        }

        this->log("BUY " + date + " " + std::to_string(price) + " " + std::to_string(quantity));
    }

    void BacktestEngine::sell(const std::string& date, double price, uint16_t quantity) {
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;

        if (!this->Position) {
            this->isShort = true;
            this->Position = true;
            this->shortQuantity = quantity;
            this->balance += (price * quantity) - _comm;
        } else if (this->isShort) {
            this->shortQuantity += quantity;
            this->balance += (price * quantity) - _comm;
        } else if (this->isLong && quantity > this->longQuantity) {
            this->isLong = false;
            this->isShort = true;
            this->shortQuantity = quantity - this->longQuantity;
            this->longQuantity = 0;
            this->balance += (price * quantity) - _comm;
        } else if (this->isLong && quantity <= this->longQuantity) {
            this->longQuantity -= quantity;
            this->balance -= (price * quantity) + _comm;
        }

        this->log("SELL " + date + " " + std::to_string(price) + " " + std::to_string(quantity));
    }

    void BacktestEngine::close(const std::string& date, double price, uint16_t quantity) {
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;

        if (this->isLong) {
            this->isLong = false;
            this->Position = false;
            this->longQuantity -= quantity;
            this->balance += (price * quantity) - _comm;
        } else if (this->isShort) {
            this->isShort = false;
            this->Position = false;
            this->shortQuantity -= quantity;
            this->balance -= (price * quantity) + _comm;
        }

        this->log("CLOSE " + date + " " + std::to_string(price) + " " + std::to_string(quantity));
    }

    void BacktestEngine::run() {
        std::cout << "Running Backtest..." << std::endl;
        std::cout << "Starting Balance: " << this->balance << std::endl;
        this->initialBalance = this->balance;
        this->next();
        if (this->isLong) {
            this->close(this->dataFeed.getOHLC().date.back(), this->dataFeed.getOHLC().close.back(), this->longQuantity);
        } else if (this->isShort) {
            this->close(this->dataFeed.getOHLC().date.back(), this->dataFeed.getOHLC().close.back(), this->shortQuantity);
        }
        this->finalBalance = this->balance;
        std::cout << "Backtest Complete." << std::endl;
        std::cout << "Final Balance: " << this->balance << std::endl;
    }

    void BacktestEngine::stats() {
        std::cout << "Stats" << std::endl;
        bool isProfitable = this->finalBalance > this->initialBalance;
        if (isProfitable) {
            std::cout << "Profit: " << this->finalBalance - this->initialBalance << std::endl;
        } else {
            std::cout << "Loss: " << this->finalBalance - this->initialBalance << std::endl;
        }
        std::cout << "Return: " << (this->finalBalance - this->initialBalance) / this->initialBalance * 100 << "%" << std::endl;
        std::cout << "Commission: " << this->totalCommission << std::endl;
    }

    void BacktestEngine::log(const std::string& message) {
        std::cout << message << std::endl;
    }

}
