#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <sstream>

enum LogLevel { DEBUG, INFO, WARN, ERROR};
static const char *logStrings[] = { "DEBUG", "INFO ", "WARN ", "ERROR" };

const LogLevel logLevel = INFO;

class logIt {
    LogLevel level;
public:
    logIt(LogLevel l);

    template <typename T>
    logIt & operator<<(T const & value) {
        _buffer << value;
        return *this;
    }

    ~logIt();

private:
    std::ostringstream _buffer;
    bool toPrint;
};

#define logger(l) logIt(l)

#endif
