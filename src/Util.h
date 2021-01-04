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
        current = str.find_first_of(delims);
        while (current != std::string::npos) {
            cont.push_back(str.substr(previous, current - previous));
            previous = current + delims.size();
            current = str.find_first_of(delims, previous);
        }
        cont.push_back(str.substr(previous, current - previous));
    }
};

#endif