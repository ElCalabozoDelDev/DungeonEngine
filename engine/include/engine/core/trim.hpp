#ifndef DE_CORE_TRIM_HPP
#define DE_CORE_TRIM_HPP

#include <algorithm>
#include <cctype>
#include <string>

namespace de
{
/// Strips every whitespace character from `str` (not just the ends): TMX tile
/// payloads arrive wrapped across lines and base64 cannot tolerate that.
inline std::string trim(const std::string& str)
{
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(),
                                [](unsigned char x)
                                { return std::isspace(x) != 0; }),
                 result.end());
    return result;
}

} // namespace de

#endif // DE_CORE_TRIM_HPP
