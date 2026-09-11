#ifndef DE_COMPONENTS_LEVEL_COMPONENT_HPP
#define DE_COMPONENTS_LEVEL_COMPONENT_HPP
#include "entt/entity/fwd.hpp"
#include <vector>

namespace de
{
struct LevelComponent
{
    std::vector<entt::entity> tilesets;
    std::vector<entt::entity> layers;
};

} // namespace de

#endif // DE_COMPONENTS_LEVEL_COMPONENT_HPP
