#ifndef DUNGEON_COMPONENT_HPP
#define DUNGEON_COMPONENT_HPP

#include "world/level.hpp"
#include "SDL.h"
#include <map>

struct LevelComponent {
    Level *level;
};

#endif // DUNGEON_COMPONENT_HPP