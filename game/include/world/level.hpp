#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "world/layer.hpp"
#include "world/tile_set.hpp"
#include "entt/entt.hpp"
#include <vector>


class Level {
private:
	friend class LevelParser;
	std::vector<TileSet> m_tilesets;
	std::vector<Layer *> m_layers;
	Level(){}

public:
	~Level() {}
	void update();
	void render(entt::registry& registry);
	std::vector<TileSet> *getTilesets() { return &m_tilesets; }
	std::vector<Layer *> *getLayers() { return &m_layers; }
};

#endif