#ifndef TRIM_HPP
#define TRIM_HPP
#include <string>

inline std::string trim(const std::string &str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(), [](unsigned char x) {
        return std::isspace(x);
    }), result.end());
    return result;
}
#endif