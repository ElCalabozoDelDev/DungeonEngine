#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "world/layer.hpp"
#include "entt/entt.hpp"
#include <vector>


class Level {
private:
	friend class LevelParser;
	std::vector<entt::entity> m_tilesets;
	std::vector<entt::entity> m_layers;
	Level(){}

public:
	~Level() {}
	std::vector<entt::entity> *getTilesets() { return &m_tilesets; }
	std::vector<entt::entity> *getLayers() { return &m_layers; }
};

#endif