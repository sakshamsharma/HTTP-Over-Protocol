#include "logger.h"

logIt::logIt(LogLevel l) {
    if(l >= logLevel) {
        _buffer << "[" << logStrings[l] << "] ";
        toPrint = true;
    } else {
        toPrint = false;
    }
}

logIt::~logIt() {
    if (toPrint) {
        _buffer << std::endl;
        std::cout << _buffer.str();
    }
}
