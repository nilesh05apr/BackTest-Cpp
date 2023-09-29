#include "BacktestEngine.h"
#include "Data.h"
#include "Indicator.h"
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

backtest::BacktestEngine::BacktestEngine(std::vector<data::OHLC> data){
    this->data = data;
    this->isLong = false;
    this->isShort = false;
    this->Position = false;

    this->quantity = 0;
    this->longQuantity = 0;
    this->shortQuantity = 0;

    this->balance = 0;
    this->commission = 0;
    this->slippage = 0;
    this->sizer = 0;

}

backtest::BacktestEngine::BacktestEngine(data::OHLC ohlc){
    this->ohlc = ohlc;
    this->isLong = false;
    this->isShort = false;
    this->Position = false;

    this->quantity = 0;
    this->longQuantity = 0;
    this->shortQuantity = 0;

    this->balance = 0;
    this->commission = 0;
    this->slippage = 0;
    this->sizer = 0;

}

void backtest::BacktestEngine::setBalance(double balance){
    this->balance = balance;
}

void backtest::BacktestEngine::setCommission(double commission){
    this->commission = commission;
    this->totalCommission = 0.0;
}

void backtest::BacktestEngine::setSlippage(double slippage){
    this->slippage = slippage;
}

void backtest::BacktestEngine::setSizer(double sizer){
    this->sizer = sizer;
}

void backtest::BacktestEngine::buy(std::string date, double price, uint16_t quantity){
    if(this->Position == false) {
        this->isLong = true;
        this->Position = true;
        this->longQuantity += quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance -= (price * quantity) + _comm;
        std::string message = "BUY " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    } else if (this->Position == true && this->isLong == true) {
        this->longQuantity += quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance -= (price * quantity) + _comm;
        std::string message = "BUY " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);        
    } else if (this->Position == true && this->isShort == true && quantity > this->shortQuantity) {
        this->isShort = false;
        this->isLong = true;
        this->longQuantity = quantity - this->shortQuantity;
        this->shortQuantity = 0;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance -= (price * quantity) + _comm;
        std::string message = "BUY " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);        
    } else if (this->Position == true && this->isShort == true && quantity < this->shortQuantity) {
        this->shortQuantity -= quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance -= (price * quantity) + _comm;
        std::string message = "BUY " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    }
}

void backtest::BacktestEngine::sell(std::string date, double price, uint16_t quantity){
    if(this->Position == false) {
        this->isShort = true;
        this->Position = true;
        this->shortQuantity += quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance += (price * quantity) - _comm;
        std::string message = "SELL " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    } else if (this->Position == true && this->isShort == true) {
        this->shortQuantity += quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance += (price * quantity) - _comm;
        std::string message = "SELL " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);        
    } else if (this->Position == true && this->isLong == true && quantity > this->longQuantity) {
        this->isLong = false;
        this->isShort = true;
        this->shortQuantity = quantity - this->longQuantity;
        this->longQuantity = 0;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance += (price * quantity) - _comm;
        std::string message = "SELL " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);        
    } else if (this->Position == true && this->isLong == true && quantity < this->longQuantity) {
        this->longQuantity -= quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance += (price * quantity) - _comm;
        std::string message = "SELL " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    }
}

void backtest::BacktestEngine::close(std::string date, double price, uint16_t quantity){
    if (this->isLong == true){
        this->isLong = false;
        this->Position = false;
        this->longQuantity -= quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance += (price * quantity) - _comm;
        std::string message = "CLOSE " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    }
    else if (this->isShort == true){
        this->isShort = false;
        this->Position = false;
        this->shortQuantity -= quantity;
        double _comm = this->commission * price * quantity;
        this->totalCommission += _comm;
        this->balance -= (price * quantity) + _comm;
        std::string message = "CLOSE " + date + " " + std::to_string(price) + " " + std::to_string(quantity);
        this->log(message);
    }
}

void backtest::BacktestEngine::run(){
    std::cout << "Running Backtest..." << std::endl;
    std::cout << "Starting Balance: " << this->balance << std::endl;
    this->initialBalance = this->balance;
    this->next();
    if(this->isLong == true){
        this->close(this->ohlc.date[this->ohlc.date.size()-1], this->ohlc.close[this->ohlc.close.size()-1], this->longQuantity);
    } else if(this->isShort == true){
        this->close(this->ohlc.date[this->ohlc.date.size()-1], this->ohlc.close[this->ohlc.close.size()-1], this->shortQuantity);
    }
    this->finalBalance = this->balance;
    std::cout << "Backtest Complete." << std::endl;
    std::cout << "Final Balance: " << this->balance << std::endl;    
}

void backtest::BacktestEngine::stats(){
    std::cout << "Stats" << std::endl;
    bool isProfitable = this->finalBalance > this->initialBalance;
    if(isProfitable){
        std::cout << "Profit::" << this->finalBalance - this->initialBalance << std::endl;
    } else {
        std::cout << "Loss::" << this->finalBalance - this->initialBalance << std::endl;
    }
    std::cout << "Return::" << (this->finalBalance - this->initialBalance) / this->initialBalance * 100 << "%" << std::endl;
    std::cout << "Commission::" << this->totalCommission << std::endl;
}

void backtest::BacktestEngine::log(std::string message){
    std::cout << message << std::endl;
}


