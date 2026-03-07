# BackTest-Cpp

A header-only, high-throughput backtesting engine in C++17. Supports CSV data
feeds, built-in technical indicators (SMA, EMA, RSI), and three complementary
backtesting modes — **batch/streaming**, **event-driven**, and **vectorized** —
all in the same framework. Includes replay from any bar index or date.

## Features
- **Core library** (header-only, under `include/backtest`):
  - `Engine`: CRTP-driven batch engine with cash, commission, slippage, and
    position management. Supports `run()`, `runFrom(barIndex)`, and
    `runFrom(startDate)` for replaying from any point in the data.
  - `EventDrivenEngine`: Event-driven engine with a built-in event queue
    (`MarketEvent → SignalEvent → OrderEvent → FillEvent`). Override
    `onMarket`, `onSignal`, `onOrder`, and `onFill` to implement the four-step
    event lifecycle. Also supports `runFrom()` replay.
  - `VectorizedEngine`: Receives the entire dataset as one `OHLCBatch` inside
    `onData()` for whole-array signal computation. Also supports `runFrom()`
    replay.
  - `Strategy`: Thin CRTP base over `Engine`.
  - `data::CSVDataFeed` & `data::OHLCBatch`: Batch reader and columnar OHLC
    storage. Exposes `reset()` to rewind the feed.
  - `indicators`: Header-only SMA, EMA, and RSI functions.
- **Examples**:
  - **SupportResistance**: Pivot-based strategy (classic batch mode).
  - **StreamingSupportResistance**: Same strategy, streaming batch mode.
  - **EventDrivenSupportResistance**: Same strategy rebuilt with the
    event-driven engine (`MarketEvent → SignalEvent → OrderEvent → FillEvent`).
  - **VectorizedSupportResistance**: Same strategy rebuilt with the vectorized
    engine; pivot levels are smoothed with a 5-period SMA before trading.

## Requirements
- C++17 compiler (GCC, Clang, MSVC)
- CMake ≥ 3.10
- Standard C++ library

## Building
```bash
# From project root
mkdir build && cd build
cmake ..
make -j4
```

Executables are generated under `build/Examples/`:
```
SupportResistance/SupportResistance
StreamingSupportResistance/StreamingSupportResistance
EventDrivenSupportResistance/EventDrivenSupportResistance
VectorizedSupportResistance/VectorizedSupportResistance
```

## Usage

### SupportResistance (batch mode)
```bash
build/Examples/SupportResistance/SupportResistance
```

### StreamingSupportResistance (streaming mode)
```bash
build/Examples/StreamingSupportResistance/StreamingSupportResistance \
  Examples/Data/^NSEMDCP50.csv [batch_size]
```

### EventDrivenSupportResistance (event-driven mode)
```bash
# Full run
build/Examples/EventDrivenSupportResistance/EventDrivenSupportResistance

# Replay from bar 50
build/Examples/EventDrivenSupportResistance/EventDrivenSupportResistance \
  Examples/Data/^NSEMDCP50.csv 1024 50

# Replay from a specific date
build/Examples/EventDrivenSupportResistance/EventDrivenSupportResistance \
  Examples/Data/^NSEMDCP50.csv 1024 2023-07-01
```

### VectorizedSupportResistance (vectorized mode)
```bash
# Full run
build/Examples/VectorizedSupportResistance/VectorizedSupportResistance

# Replay from bar 100
build/Examples/VectorizedSupportResistance/VectorizedSupportResistance \
  Examples/Data/^NSEMDCP50.csv 1024 100

# Replay from a specific date
build/Examples/VectorizedSupportResistance/VectorizedSupportResistance \
  Examples/Data/^NSEMDCP50.csv 1024 2023-06-01
```

## Core API Overview

### Batch / Streaming Engine (`backtest::Engine<Derived, Feed>`)
```cpp
strat.setCash(100000);
strat.setCommission(0.001);

strat.run();                        // run from the first bar
strat.runFrom(50);                  // skip the first 50 bars
strat.runFrom("2023-07-01");        // skip bars before this date
```
Override in your strategy:
- `onStart()` / `onEnd()`
- `onBar(const OHLCBatch& batch, std::size_t idx)`
- `buy(date, price, qty)` / `sell(...)` / `close(...)`

### Event-Driven Engine (`backtest::EventDrivenEngine<Derived, Feed>`)
```cpp
strat.run();
strat.runFrom(50);
strat.runFrom("2023-07-01");
```
Override in your strategy:
- `onStart()` / `onEnd()`
- `onMarket(const MarketEvent& evt)` → call `pushSignal(...)`
- `onSignal(const SignalEvent& evt)` → call `pushOrder(...)`
- `onOrder (const OrderEvent&  evt)` → call `pushFill(...)`
- `onFill  (const FillEvent&   evt)` → call `executeBuy/Sell/Close(...)`

Event types: `MarketEvent`, `SignalEvent`, `OrderEvent`, `FillEvent`
Signal directions: `SignalDirection::LONG`, `SHORT`, `EXIT`
Order directions: `OrderDirection::BUY`, `SELL`
Order types: `OrderType::MARKET`, `LIMIT`

### Vectorized Engine (`backtest::VectorizedEngine<Derived, Feed>`)
```cpp
strat.run();
strat.runFrom(50);
strat.runFrom("2023-07-01");
```
Override in your strategy:
- `onStart()` / `onEnd()`
- `onData(const OHLCBatch& batch)` — receives the **entire** dataset at once
- `recordTrade(date, "BUY"|"SELL", price, qty)` — updates balance and logs

### Data Feed (`data::CSVDataFeed`)
```cpp
data::CSVDataFeed feed("data.csv", /*batchSize=*/1024);
feed.reset();   // rewind to the first data bar
```

### Indicators (`backtest::indicators`)
```cpp
auto sma = backtest::indicators::SMA(prices, 20);
auto ema = backtest::indicators::EMA(prices, 20);
auto rsi = backtest::indicators::RSI(prices, 14);
```

## Directory Structure
```
.                                    # Project root
├── CMakeLists.txt
├── include/backtest/
│   ├── Engine.h                     # Batch / streaming engine
│   ├── EventDrivenEngine.h          # Event-driven engine
│   ├── VectorizedEngine.h           # Vectorized engine
│   ├── Strategy.h                   # CRTP strategy base
│   ├── Data.h                       # CSVDataFeed
│   ├── FileIterator.h               # CSV batch reader
│   └── Indicators.h                 # SMA / EMA / RSI
├── Examples/
│   ├── Data/                        # Sample CSV files
│   ├── SupportResistance/
│   ├── StreamingSupportResistance/
│   ├── EventDrivenSupportResistance/
│   └── VectorizedSupportResistance/
└── build/                           # CMake build output
```

## Contributing
Contributions are welcome! Please open issues or pull requests for enhancements or bug fixes.