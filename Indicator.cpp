#include "Indicator.h"
#include "Data.h"
#include <vector>
#include <stdint.h>

Indicators::Indicators(std::vector<data::OHLC> data){
    this->data = data;
}

Indicators::Indicators(data::OHLC ohlc){
    this->ohlc = ohlc;
}

std::vector<double> Indicators::SMA(int period){
    std::vector<double> sma(this->ohlc.close.size(), 0.0);
    std::vector<double> data = this->ohlc.close;
    double sum = 0;
    for (int i = 0; i < period; i++){
        sum += data[i];
    }
    sma[period - 1] = sum / period;
    for (int i = period; i < data.size(); i++){
        sum -= data[i - period];
        sum += data[i];
        sma[i] = sum / period;
    }
    return sma;
}

std::vector<double> Indicators::EMA(int period){
    std::vector<double> ema(this->ohlc.close.size(), 0.0);
    std::vector<double> data = this->ohlc.close;
    double sum = 0;
    for (int i = 0; i < period; i++){
        sum += data[i];
    }
    ema[period - 1] = sum / period;
    for (int i = period; i < data.size(); i++){
        ema[i] = (data[i] * (2 / (period + 1))) + (ema[i - 1] * (1 - (2 / (period + 1))));
    }
    return ema;
}

std::vector<double> Indicators::ADX(int period){
    std::vector<double> data = this->ohlc.close;
    std::vector<double> adx(data.size());
    std::vector<double> tr(data.size());
    std::vector<double> plusDM(data.size());
    std::vector<double> minusDM(data.size());
    std::vector<double> plusDI(data.size());
    std::vector<double> minusDI(data.size());
    std::vector<double> dx(data.size());
    std::vector<double> adxr(data.size());
    std::vector<double> atr(data.size());
    return adx;
}