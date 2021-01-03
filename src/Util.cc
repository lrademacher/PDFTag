#include "Util.h"
#include <algorithm>

void Util::tolower(std::string &in)
{
    std::transform(in.begin(), in.end(), in.begin(),
    [](unsigned char c){ return std::tolower(c); });
}