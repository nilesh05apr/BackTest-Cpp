// Indicators.h: header-only technical indicators
#ifndef BACKTEST_INDICATORS_H
#define BACKTEST_INDICATORS_H

#include <vector>
#include <cstddef>
#include <numeric>
#include <cmath>

namespace backtest {
namespace indicators {

// Simple Moving Average
inline std::vector<double> SMA(const std::vector<double>& data, std::size_t period) {
    std::vector<double> sma(data.size(), 0.0);
    if (data.size() < period || period == 0) return sma;
    double sum = std::accumulate(data.begin(), data.begin() + period, 0.0);
    sma[period - 1] = sum / period;
    for (std::size_t i = period; i < data.size(); ++i) {
        sum += data[i] - data[i - period];
        sma[i] = sum / period;
    }
    return sma;
}

// Exponential Moving Average
inline std::vector<double> EMA(const std::vector<double>& data, std::size_t period) {
    std::vector<double> ema(data.size(), 0.0);
    if (data.size() < period || period == 0) return ema;
    // Start EMA with SMA for first period
    double sum = std::accumulate(data.begin(), data.begin() + period, 0.0);
    double prev = sum / period;
    ema[period - 1] = prev;
    double alpha = 2.0 / (period + 1);
    for (std::size_t i = period; i < data.size(); ++i) {
        prev = alpha * (data[i] - prev) + prev;
        ema[i] = prev;
    }
    return ema;
}

// Relative Strength Index
inline std::vector<double> RSI(const std::vector<double>& data, std::size_t period) {
    std::vector<double> rsi(data.size(), 0.0);
    if (data.size() <= period || period == 0) return rsi;
    double gain = 0.0, loss = 0.0;
    for (std::size_t i = 1; i <= period; ++i) {
        double delta = data[i] - data[i - 1];
        if (delta >= 0) gain += delta;
        else loss += -delta;
    }
    gain /= period;
    loss /= period;
    if (loss == 0) rsi[period] = 100.0;
    else {
        double rs = gain / loss;
        rsi[period] = 100.0 - 100.0 / (1 + rs);
    }
    for (std::size_t i = period + 1; i < data.size(); ++i) {
        double delta = data[i] - data[i - 1];
        double g = (delta > 0) ? delta : 0.0;
        double l = (delta < 0) ? -delta : 0.0;
        gain = (gain * (period - 1) + g) / period;
        loss = (loss * (period - 1) + l) / period;
        if (loss == 0) rsi[i] = 100.0;
        else {
            double rs = gain / loss;
            rsi[i] = 100.0 - 100.0 / (1 + rs);
        }
    }
    return rsi;
}

} // namespace indicators
} // namespace backtest
#endif // BACKTEST_INDICATORS_H