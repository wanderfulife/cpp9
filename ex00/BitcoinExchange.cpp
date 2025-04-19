#include "BitcoinExchange.hpp"
#include <cstdlib> 
#include <algorithm>
#include <fstream>
#include <sstream>
#include <limits>
#include <iomanip>

BitcoinExchange::BitcoinExchange(const BitcoinExchange& other) {
    (void)other;
}

BitcoinExchange& BitcoinExchange::operator=(const BitcoinExchange& other) {
    (void)other;
    return *this;
}

void BitcoinExchange::loadDatabase(const std::string& filename) {
    std::ifstream dbFile(filename.c_str());
    if (!dbFile.is_open()) {
        throw CouldNotOpenFileException();
    }

    std::string line;
    if (!std::getline(dbFile, line)) {
        dbFile.close();
        throw std::runtime_error("Database file is empty or header is missing.");
    }
    if (line != "date,exchange_rate") {
        std::cerr << "Warning: Unexpected CSV header: " << line << std::endl;
    }

    while (std::getline(dbFile, line)) {
        std::stringstream ss(line);
        std::string dateStr;
        std::string rateStr;
        double rate;

        if (std::getline(ss, dateStr, ',') && std::getline(ss, rateStr)) {
            if (dateStr.length() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
                 std::cerr << "Warning: Invalid date format in database: " << dateStr << std::endl;
                 continue;
            }

            char* endPtr;
            rate = std::strtod(rateStr.c_str(), &endPtr);
            if (*endPtr != '\0' || rateStr.empty()) {
                std::cerr << "Warning: Invalid rate format in database for date " << dateStr << ": " << rateStr << std::endl;
                continue;
            }
            if (rate < 0) {
                 std::cerr << "Warning: Negative rate in database for date " << dateStr << ": " << rate << std::endl;
                 continue;
            }

            _database[dateStr] = rate;
        } else {
            std::cerr << "Warning: Invalid line format in database: " << line << std::endl;
        }
    }

    dbFile.close();
    if (_database.empty()) {
         throw std::runtime_error("Database file contained no valid data.");
    }
}

bool BitcoinExchange::isValidDate(const std::string& dateStr) const {
    if (dateStr.length() != 10) return false;
    if (dateStr[4] != '-' || dateStr[7] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(dateStr[i])) return false;
    }

    int year = std::atoi(dateStr.substr(0, 4).c_str());
    int month = std::atoi(dateStr.substr(5, 2).c_str());
    int day = std::atoi(dateStr.substr(8, 2).c_str());

    if (year < 0) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;

    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        return false;
    }
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (isLeap && day > 29) {
            return false;
        }
        if (!isLeap && day > 28) {
            return false;
        }
    }

    return true;
}

bool BitcoinExchange::isValidValue(const std::string& valueStr, float& value) const {
    char* endPtr;
    value = std::strtof(valueStr.c_str(), &endPtr);

    if (*endPtr != '\0' || valueStr.empty()) {
        return false;
    }

    if (value == std::numeric_limits<float>::infinity() || value == -std::numeric_limits<float>::infinity()) {
         return false;
    }

    if (value < 0) {
        return false;
    }
    if (value > 1000) {
        return false;
    }

    return true;
}

double BitcoinExchange::getRate(const std::string& date) const {
    std::map<std::string, double>::const_iterator it = _database.find(date);
    if (it != _database.end()) {
        return it->second;
    }

    it = _database.upper_bound(date);

    if (it == _database.begin()) {
        throw std::runtime_error("No data available for or prior to this date.");
    }

    --it;
    return it->second;
}

BitcoinExchange::BitcoinExchange() {
    try {
        loadDatabase("data.csv");
    } catch (const std::exception& e) {
        std::cerr << "Error loading database: " << e.what() << std::endl;
    }
}

BitcoinExchange::~BitcoinExchange() {}

void BitcoinExchange::processInput(const std::string& filename) {
    std::ifstream inputFile(filename.c_str());
    if (!inputFile.is_open()) {
        std::cerr << "Error: could not open file." << std::endl;
        return;
    }
    if (_database.empty()) {
        std::cerr << "Error: Database is not loaded or empty." << std::endl;
        inputFile.close();
        return;
    }

    std::string line;
    if (!std::getline(inputFile, line)) {
        std::cerr << "Error: Input file is empty." << std::endl;
        inputFile.close();
        return;
    }

    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string dateStr;
        std::string separator;
        std::string valueStr;
        float value;

        if (!(ss >> dateStr >> separator >> valueStr) || separator != "|") {
             std::string trimmedLine = line;
             trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t\n\r"));
             trimmedLine.erase(trimmedLine.find_last_not_of(" \t\n\r") + 1);
             if (!trimmedLine.empty()) {
                std::cerr << "Error: bad input => " << line << std::endl;
             }
            continue;
        }

        std::string remaining;
        if (ss >> remaining) {
             std::cerr << "Error: bad input => " << line << std::endl;
             continue;
        }

        dateStr.erase(0, dateStr.find_first_not_of(" \t"));
        dateStr.erase(dateStr.find_last_not_of(" \t") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t"));
        valueStr.erase(valueStr.find_last_not_of(" \t") + 1);

        if (!isValidDate(dateStr)) {
            std::cerr << "Error: bad input => " << dateStr << std::endl;
            continue;
        }

        if (!isValidValue(valueStr, value)) {
            char* endPtr;
            double checkValue = std::strtod(valueStr.c_str(), &endPtr);
             if (*endPtr != '\0' || valueStr.empty()) {
                 std::cerr << "Error: bad input => " << line << std::endl;
             }
             else if (checkValue < 0) {
                std::cerr << "Error: not a positive number." << std::endl;
            } else if (checkValue > 1000) {
                std::cerr << "Error: too large a number." << std::endl;
            } else {
                 std::cerr << "Error: bad input => " << line << std::endl;
            }
            continue;
        }

        try {
            double rate = getRate(dateStr);
            double result = value * rate;
            std::cout << dateStr << " => " << value << " = " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << " (for date: " << dateStr << ")" << std::endl;
        }
    }

    inputFile.close();
}


const char* BitcoinExchange::CouldNotOpenFileException::what() const throw() {
    return "could not open file.";
}

const char* BitcoinExchange::BadInputException::what() const throw() {
    return "bad input =>";
}

const char* BitcoinExchange::InvalidDateException::what() const throw() {
    return "invalid date format.";
}

const char* BitcoinExchange::InvalidValueException::what() const throw() {
    return "invalid value format.";
}

const char* BitcoinExchange::NegativeNumberException::what() const throw() {
    return "not a positive number.";
}

const char* BitcoinExchange::TooLargeNumberException::what() const throw() {
    return "too large a number.";
}
