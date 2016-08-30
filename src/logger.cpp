#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <iostream>
#include <sstream>

/* consider adding boost thread id since we'll want to know whose writting and
 * won't want to repeat it for every single call */

/* consider adding policy class to allow users to redirect logging to specific
 * files via the command line
 */

class logIt {
public:
    logIt() {
#ifdef DEBUG
        _buffer << "[DEBUG] ";
        toPrint = true;
#else
        toPrint = false;
#endif
    }

    template <typename T>
    logIt & operator<<(T const & value) {
        _buffer << value;
        return *this;
    }

    ~logIt() {
        if (toPrint) {
            _buffer << std::endl;
            std::cout << _buffer.str();
        }
    }

private:
    std::ostringstream _buffer;
    bool toPrint;
};

#define logger \
    logIt()
#endif
