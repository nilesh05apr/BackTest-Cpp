#include "Data.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace data {

    void DataFeed::readCSV(const std::string& filename) {
        const std::vector<std::string> _columns = {"Date", "Open", "High", "Low", "Close", "Adj Close", "Volume"};

        // Read file and check if it is open
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file");
        }

        std::string header;
        if (std::getline(file, header)) {
            std::stringstream headerStream(header);
            std::string column;
            std::vector<std::string> columns;

            while (std::getline(headerStream, column, ',')) {
                columns.push_back(column);
            }

            for (const auto& column : _columns) {
                if (std::find(columns.begin(), columns.end(), column) == columns.end()) {
                    throw std::runtime_error("Invalid CSV file");
                }
            }

            // Read and process the data lines
            std::string line;
            while (std::getline(file, line)) {
                parseLine(line);
            }
        } else {
            std::cerr << "Empty file." << std::endl;
        }
        file.close();
    }

    void DataFeed::parseLine(const std::string& line) {
        std::stringstream lineStream(line);
        std::string value;

        if (std::getline(lineStream, value, ',')) {
            ohlc.date.push_back(value);
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.open.push_back(std::stod(value));
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.high.push_back(std::stod(value));
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.low.push_back(std::stod(value));
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.close.push_back(std::stod(value));
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.adjClose.push_back(std::stod(value));
        }
        if (std::getline(lineStream, value, ',')) {
            ohlc.volume.push_back(std::stod(value));
        }
    }

    void DataFeed::printOHLC() const {
        for (size_t i = 0; i < ohlc.date.size(); i++) {
            std::cout << ohlc.date[i] << " "
                      << ohlc.open[i] << " " 
                      << ohlc.high[i] << " " 
                      << ohlc.low[i] << " " 
                      << ohlc.close[i] << " "
                      << ohlc.adjClose[i] << " "
                      << ohlc.volume[i] 
                      << std::endl;
        }
    }

    void DataFeed::printOHLC(uint16_t index) const {
        if (index < ohlc.date.size()) {
            std::cout << ohlc.date[index] << " " 
                      << ohlc.open[index] << " " 
                      << ohlc.high[index] << " " 
                      << ohlc.low[index] << " " 
                      << ohlc.close[index] << " "
                      << ohlc.adjClose[index] << " "
                      << ohlc.volume[index] 
                      << std::endl;
        } else {
            std::cerr << "Index out of range." << std::endl;
        }
    }

    void DataFeed::printOHLC(uint16_t start, uint16_t end) const {
        if (start < end && end <= ohlc.date.size()) {
            for (uint16_t i = start; i < end; i++) {
                std::cout << ohlc.date[i] << " " 
                          << ohlc.open[i] << " " 
                          << ohlc.high[i] << " " 
                          << ohlc.low[i] << " " 
                          << ohlc.close[i] << " "
                          << ohlc.adjClose[i] << " "
                          << ohlc.volume[i] 
                          << std::endl;
            }
        } else {
            std::cerr << "Invalid range." << std::endl;
        }
    }

    void DataFeed::clearData() {
        ohlc = OHLC();
    }

    const OHLC& DataFeed::getOHLC() const {
        return ohlc;
    }

    size_t DataFeed::size() const {
        return ohlc.date.size();
    }

}