#ifndef __UTIL_H
#define __UTIL_H

#include <string>

class Util {
public:
    static void tolower(std::string &in);

    template <class Container>
    static void splitString(const std::string& str, Container& cont,
                    const std::string& delims = " ")
    {
        std::size_t current, previous = 0;
        do {
            current = str.find(delims, previous);
            if ((current != std::string::npos) || (previous > 0))
            {
                cont.push_back(str.substr(previous, current - previous));
            }
            previous = current + delims.size();
        } while (current != std::string::npos);
    }
};

#endif