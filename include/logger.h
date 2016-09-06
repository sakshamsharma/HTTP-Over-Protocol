#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <sstream>

enum LogLevel { DEBUG, VERB1, INFO, WARN, ERROR};
static const char *logStrings[] =
    { "DEBUG", "VERB1", "INFO ", "WARN ", "ERROR" };

const LogLevel logLevel = VERB1;

class logIt {
    LogLevel level;
public:
    logIt(LogLevel l);
    logIt(LogLevel l, const char *s);

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

#define GET_MACRO(_1,_2,NAME,...) NAME
#define logger(...) GET_MACRO(__VA_ARGS__, logIt, logIt)(__VA_ARGS__)

#endif
