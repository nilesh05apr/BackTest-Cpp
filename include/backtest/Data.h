#ifndef BACKTEST_DATA_H
#define BACKTEST_DATA_H

#include <vector>
#include <string>
#include <cstddef>
#include "backtest/FileIterator.h"

namespace data {


class DataFeed {
public:
    virtual ~DataFeed() = default;
    virtual bool nextBatch() = 0;
    virtual const OHLCBatch& currentBatch() const = 0;
};

class CSVDataFeed : public DataFeed {
public:
    CSVDataFeed(const std::string& filePath, std::size_t batchSize) noexcept
        : fileIterator_(filePath, batchSize) {}
    bool nextBatch() override {
        return fileIterator_.nextBatch();
    }
    const OHLCBatch& currentBatch() const override {
        return fileIterator_.batch();
    }
private:
    FileIterator fileIterator_;
};

}
#endif // BACKTEST_DATA_H