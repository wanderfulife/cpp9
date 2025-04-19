#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>

class BitcoinExchange {
public:
    BitcoinExchange();
    ~BitcoinExchange();

    void processInput(const std::string& filename);

    class CouldNotOpenFileException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class BadInputException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class InvalidDateException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class InvalidValueException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class NegativeNumberException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class TooLargeNumberException : public std::exception {
    public:
        virtual const char* what() const throw();
    };

private:
    std::map<std::string, double> _database;

    BitcoinExchange(const BitcoinExchange& other);
    BitcoinExchange& operator=(const BitcoinExchange& other);

    void loadDatabase(const std::string& filename);
    bool isValidDate(const std::string& dateStr) const;
    bool isValidValue(const std::string& valueStr, float& value) const;
    double getRate(const std::string& date) const;

};

#endif
