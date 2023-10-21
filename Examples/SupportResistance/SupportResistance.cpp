#include "SupportResistance.h"
#include "../../Data.h"
#include "../../Indicator.h"
#include "../../Strategy.h"
#include <vector>


SupportResistance::SupportResistance(data::OHLC ohlc) : strategy::Strategy(ohlc) {
    this->ohlc = ohlc;
}

void SupportResistance::next() {
    // C = (H + L + C) / 3 of the previous day
    for(int i = 0; i < ohlc.close.size(); i++){
        this->C.push_back((ohlc.high[i] + ohlc.low[i] + ohlc.close[i]) / 3);
    }
    // Calculate support and resistance
    // Support = (2 * C) - H
    for(int i = 0; i < ohlc.close.size(); i++){
        this->support.push_back((2 * C[i]) - ohlc.high[i]);
    }
    // Resistance = (2 * C) - L
    for(int i = 0; i < ohlc.close.size(); i++){
        this->resistance.push_back((2 * C[i]) - ohlc.low[i]);
    }

    // Buy when the price crosses above the C and exit when the price above the resistance
    // Sell when the price crosses below the C and exit when the price below the support
    for(int i = 1; i < this->ohlc.close.size(); i++){
        bool longSignal = this->ohlc.open[i] > this->C[i-1];
        bool exitLong = this->ohlc.open[i] >= this->resistance[i-1];
        bool shortSignal = this->ohlc.open[i] < this->C[i-1];
        bool exitShort = this->ohlc.open[i] <= this->support[i-1];

        if(longSignal){
            this->buy(this->ohlc.date[i], this->ohlc.open[i], 1);
        } else if(exitLong){
            this->close(this->ohlc.date[i], this->ohlc.open[i], 1);
        } else if(shortSignal){
            this->sell(this->ohlc.date[i], this->ohlc.open[i], 1);
        } else if(exitShort){
            this->close(this->ohlc.date[i], this->ohlc.open[i], 1);
        }
    }
}