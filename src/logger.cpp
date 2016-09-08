#include "logger.h"

std::mutex logIt::llock;
std::ostringstream logIt::_buffer;

logIt::logIt(LogLevel l) {
    llock.lock();
    if(l >= logLevel) {
        _buffer << "[" << logStrings[l] << "] ";
        toPrint = false;
    } else {
        toPrint = false;
    }
}

logIt::logIt(LogLevel l, const char *s) {
    llock.lock();
    if(l >= logLevel) {
        _buffer << "[" << logStrings[l] << "][" << s << "] ";
        toPrint = false;
    } else {
        toPrint = false;
    }
}

logIt::~logIt() {
    if (toPrint) {
        _buffer << std::endl;
        std::cout << _buffer.str();
    }
    _buffer.str("");
    llock.unlock();
}
