// FileIterator: reads CSV in batches, storing columnar OHLCBatch
#ifndef BACKTEST_FILEITERATOR_H
#define BACKTEST_FILEITERATOR_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstddef>

namespace data {

struct OHLCBatch {
    std::vector<std::string> date;
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<double> adjClose;
    std::vector<double> volume;
    void clear() {
        date.clear(); open.clear(); high.clear(); low.clear();
        close.clear(); adjClose.clear(); volume.clear();
    }
    std::size_t size() const { return date.size(); }
};

class FileIterator {
public:
    FileIterator(const std::string& filename, std::size_t batchSize = 1024)
        : file_(filename), batchSize_(batchSize) {
        if (!file_.is_open()) throw std::runtime_error("Cannot open file " + filename);
        std::string header;
        if (!std::getline(file_, header)) throw std::runtime_error("Empty file " + filename);
        batch_.date.reserve(batchSize_);
        batch_.open.reserve(batchSize_);
        batch_.high.reserve(batchSize_);
        batch_.low.reserve(batchSize_);
        batch_.close.reserve(batchSize_);
        batch_.adjClose.reserve(batchSize_);
        batch_.volume.reserve(batchSize_);
    }

    bool nextBatch() {
        batch_.clear();
        std::string line;
        while (batch_.date.size() < batchSize_ && std::getline(file_, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string token;
            std::getline(ss, token, ','); batch_.date.push_back(token);
            std::getline(ss, token, ','); batch_.open.push_back(std::stod(token));
            std::getline(ss, token, ','); batch_.high.push_back(std::stod(token));
            std::getline(ss, token, ','); batch_.low.push_back(std::stod(token));
            std::getline(ss, token, ','); batch_.close.push_back(std::stod(token));
            std::getline(ss, token, ','); batch_.adjClose.push_back(std::stod(token));
            std::getline(ss, token, ','); batch_.volume.push_back(std::stod(token));
        }
        return !batch_.date.empty();
    }

    const OHLCBatch& batch() const noexcept { return batch_; }

    void reset() {
        file_.clear(); file_.seekg(0);
        std::string header; std::getline(file_, header);
    }

private:
    std::ifstream file_;
    std::size_t batchSize_;
    OHLCBatch batch_;
};

}
#endif // BACKTEST_FILEITERATOR_H