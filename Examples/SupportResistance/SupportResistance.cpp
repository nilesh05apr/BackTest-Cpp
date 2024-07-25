#include "SupportResistance.h"
#include "../../src/DataReader/Data.h"
#include "../../src/Indicators/Indicator.h"
#include "../../src/Strategy/Strategy.h"
#include <vector>


SupportResistance::SupportResistance(const data::DataFeed& dataFeed) : strategy::Strategy(dataFeed) {
    this->dataFeed = dataFeed;
}

void SupportResistance::next() {
    // C = (H + L + C) / 3 of the previous day
    for(int i = 0; i < dataFeed.getOHLC().close.size(); i++){
        this->C.push_back((dataFeed.getOHLC().high[i] + dataFeed.getOHLC().low[i] + dataFeed.getOHLC().close[i]) / 3);
    }
    // Calculate support and resistance
    // Support = (2 * C) - H
    for(int i = 0; i < dataFeed.getOHLC().close.size(); i++){
        this->support.push_back((2 * C[i]) - dataFeed.getOHLC().high[i]);
    }
    // Resistance = (2 * C) - L
    for(int i = 0; i < dataFeed.getOHLC().close.size(); i++){
        this->resistance.push_back((2 * C[i]) - dataFeed.getOHLC().low[i]);
    }

    // Buy when the price crosses above the C and exit when the price above the resistance
    // Sell when the price crosses below the C and exit when the price below the support
    for(int i = 1; i < this->dataFeed.getOHLC().close.size(); i++){
        bool longSignal = this->dataFeed.getOHLC().open[i] > this->C[i-1];
        bool exitLong = this->dataFeed.getOHLC().open[i] >= this->resistance[i-1];
        bool shortSignal = this->dataFeed.getOHLC().open[i] < this->C[i-1];
        bool exitShort = this->dataFeed.getOHLC().open[i] <= this->support[i-1];

        if(longSignal){
            this->buy(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
        } else if(exitLong){
            this->close(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
        } else if(shortSignal){
            this->sell(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
        } else if(exitShort){
            this->close(this->dataFeed.getOHLC().date[i], this->dataFeed.getOHLC().open[i], 1);
        }
    }
}