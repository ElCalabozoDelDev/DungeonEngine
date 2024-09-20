#ifndef DUNGEON_COMPONENT_HPP
#define DUNGEON_COMPONENT_HPP

#include <vector>
#include "entt/entity/fwd.hpp"

struct LevelComponent {
    std::vector<entt::entity> tilesets;
	std::vector<entt::entity> layers;
};

#endif // DUNGEON_COMPONENT_HPP