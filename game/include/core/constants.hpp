#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
struct LayerType {
    constexpr static const char* BOTTOM = "bottom";
    constexpr static const char* OVERLAY = "overlay";
    constexpr static const char* COLLISION = "collision";
    constexpr static const char* OBJECT = "object";
    constexpr static const char* COLLIDABLE = "collidable";
};

#endif // CONSTANTS_HPP