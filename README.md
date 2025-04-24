# BackTest-Cpp

A header-only, high-throughput backtesting engine in C++17. Supports CSV data feeds, built-in technical indicators (SMA, EMA, RSI), and simple strategy templates. Includes example projects demonstrating support/resistance breakout strategies in both batch and streaming modes.

## Features
- **Core library** (header-only, under `include/backtest`):
  - `Engine`: CRTP-driven backtest engine with cash, commission, slippage, and position management.
  - `Strategy`: Base class for user-defined strategies.
  - `data::CSVDataFeed` & `data::OHLCBatch`: Batch reader and container for OHLC CSV data.
  - `indicators`: Header-only SMA, EMA, and RSI functions.
- **Examples**:
  - **SupportResistance**: Pivot-based support/resistance strategy (batch mode).
  - **StreamingSupportResistance**: Streaming example using the same strategy on live data feed.

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

Executables are generated under:
- `build/Examples/SupportResistance/SupportResistance`
- `build/Examples/StreamingSupportResistance/StreamingSupportResistance`

## Usage

### SupportResistance (batch mode)
```bash
build/Examples/SupportResistance/SupportResistance
```

### StreamingSupportResistance (streaming mode)
```bash
# Provide CSV path and optional batch size
build/Examples/StreamingSupportResistance/StreamingSupportResistance \
  ../Examples/Data/^NSEMDCP50.csv [batch_size]
```

## Core API Overview
- **Engine** (`backtest::Engine<Derived, Feed>`):
  - `setCash(double)`, `setCommission(double)`, `setSlippage(double)`
  - `run()`: invokes `onStart()`, `onBar(batch, idx)`, `onEnd()` on the derived strategy.
  - `buy()`, `sell()`, `close()`: position operations with commission tracking.
- **Data Feed**:
  - `data::CSVDataFeed(filePath, batchSize)`: loads CSV in batches.
  - `data::OHLCBatch`: columnar storage for date, open, high, low, close, adjClose, volume.
- **Indicators** (`backtest::indicators`):
  - `SMA(data, period)`, `EMA(data, period)`, `RSI(data, period)`

## Directory Structure
```
.                      # Project root
├── CMakeLists.txt    # Top-level CMake configuration
├── include/          # Header-only backtest core
│   └── backtest/
├── Examples/         # Example strategies and data
│   ├── SupportResistance/
│   └── StreamingSupportResistance/
└── build/            # Build output (CMake / Makefiles)
```

## Contributing
Contributions are welcome! Please open issues or pull requests for enhancements or bug fixes.