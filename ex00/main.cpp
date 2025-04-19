#include "BitcoinExchange.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    std::string inputFilename = argv[1];

    try {
        BitcoinExchange btcExchange;
        btcExchange.processInput(inputFilename);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
