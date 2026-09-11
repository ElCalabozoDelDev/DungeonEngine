#ifndef DE_CORE_CONSTANTS_HPP
#define DE_CORE_CONSTANTS_HPP

namespace de
{
struct LayerType
{
    constexpr static const char* BOTTOM = "bottom";
    constexpr static const char* OVERLAY = "overlay";
    constexpr static const char* COLLISION = "collision";
    constexpr static const char* OBJECT = "object";
};

} // namespace de

#endif // DE_CORE_CONSTANTS_HPP
