#include "Indicator.h"
#include "../DataReader/Data.h"
#include <vector>
#include <stdint.h>
#include <cmath>

Indicators::Indicators(const data::DataFeed& dataFeed){
    this->dataFeed = dataFeed;
}

std::vector<double> Indicators::SMA(int period){
    std::vector<double> sma(this->dataFeed.getOHLC().close.size(), 0.0);
    std::vector<double> data = this->dataFeed.getOHLC().close;
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
    std::vector<double> ema(this->dataFeed.getOHLC().close.size(), 0.0);
    std::vector<double> data = this->dataFeed.getOHLC().close;
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
    std::vector<double> data = this->dataFeed.getOHLC().close;
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

std::vector<double> Indicators::RSI(int period){
    std::vector<double> rsi(this->dataFeed.getOHLC().close.size(), 0.0);
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> gain(data.size(), 0.0);
    std::vector<double> loss(data.size(), 0.0);
    gain[0] = 0;
    loss[0] = 0;
    for (int i = 1; i < data.size(); i++){
        if (data[i] > data[i - 1]){
            gain[i] = data[i] - data[i - 1];
            loss[i] = 0;
        }
        else{
            gain[i] = 0;
            loss[i] = data[i - 1] - data[i];
        }
    }
    for (int i = period; i < data.size(); i++){
        double avgGain = 0;
        double avgLoss = 0;
        for (int j = i - period; j < i; j++){
            avgGain += gain[j];
            avgLoss += loss[j];
        }
        avgGain /= period;
        avgLoss /= period;
        rsi[i] = 100 - (100 / (1 + (avgGain / avgLoss)));
    }
    return rsi;
}

std::vector<double> Indicators::MACD(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> macd(data.size());
    std::vector<double> ema12 = EMA(12);
    std::vector<double> ema26 = EMA(26);
    for (int i = 0; i < data.size(); i++){
        macd[i] = ema12[i] - ema26[i];
    }
    return macd;
}

std::vector<double> Indicators::SIGNAL(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> signal(data.size());
    std::vector<double> macd = MACD(9);
    for (int i = 0; i < data.size(); i++){
        signal[i] = macd[i] - macd[i - 1];
    }
    return signal;
}

std::vector<double> Indicators::ATR(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> atr(data.size());
    for (int i = 0; i < period; i++){
        atr[i] = 0;
    }
    for (int i = period; i < data.size(); i++){
        double tr = std::max(data[i] - data[i - 1], std::max(data[i] - this->dataFeed.getOHLC().high[i - 1], this->dataFeed.getOHLC().low[i - 1] - data[i]));
        atr[i] = (atr[i - 1] * (period - 1) + tr) / period;
    }
    return atr;
}

std::vector<double> Indicators::BOLLINGER(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> bollinger(data.size());
    std::vector<double> sma = SMA(period);
    std::vector<double> stdDev(data.size());
    for (int i = 0; i < period; i++){
        stdDev[i] = 0;
    }
    for (int i = period; i < data.size(); i++){
        double sum = 0;
        for (int j = i - period; j < i; j++){
            sum += (data[j] - sma[i]) * (data[j] - sma[i]);
        }
        stdDev[i] = std::sqrt(sum / period);
    }
    for (int i = 0; i < data.size(); i++){
        bollinger[i] = (data[i] - sma[i]) / (2 * stdDev[i]);
    }
    return bollinger;
}

std::vector<double> Indicators::STOCHASTIC(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> stochastic(data.size());
    for (int i = 0; i < period; i++){
        stochastic[i] = 0;
    }
    for (int i = period; i < data.size(); i++){
        double min = data[i - period];
        double max = data[i - period];
        for (int j = i - period; j < i; j++){
            if (data[j] < min){
                min = data[j];
            }
            if (data[j] > max){
                max = data[j];
            }
        }
        stochastic[i] = (data[i] - min) / (max - min);
    }
    return stochastic;
}

std::vector<double> Indicators::WILLIAMS(int period){
    std::vector<double> data = this->dataFeed.getOHLC().close;
    std::vector<double> williams(data.size());
    for (int i = 0; i < period; i++){
        williams[i] = 0;
    }
    for (int i = period; i < data.size(); i++){
        double min = data[i - period];
        double max = data[i - period];
        for (int j = i - period; j < i; j++){
            if (data[j] < min){
                min = data[j];
            }
            if (data[j] > max){
                max = data[j];
            }
        }
        williams[i] = (max - data[i]) / (max - min);
    }
    return williams;
}