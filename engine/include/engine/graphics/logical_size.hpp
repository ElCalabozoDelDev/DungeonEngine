#ifndef DE_GRAPHICS_LOGICAL_SIZE_HPP
#define DE_GRAPHICS_LOGICAL_SIZE_HPP

#include <SDL.h>
#include <engine/loaders/config.hpp>
#include <entt/entt.hpp>
#include <utility>

namespace de
{
/// Logical canvas size (SDL_RenderSetLogicalSize / Config cameraWidth×Height).
inline std::pair<float, float> logicalSize(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>(); config != nullptr)
    {
        return {config->cameraWidth, config->cameraHeight};
    }
    return {320.0f, 180.0f};
}

inline float windowToLogicalScale(entt::registry& registry)
{
    if (const auto* config = registry.ctx().find<Config>();
        config != nullptr && config->cameraWidth > 0.0f)
    {
        return static_cast<float>(config->screenWidth) / config->cameraWidth;
    }
    return 4.0f;
}

} // namespace de

#endif // DE_GRAPHICS_LOGICAL_SIZE_HPP
