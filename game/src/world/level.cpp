#include "world/level.hpp"

void Level::render(entt::registry& registry) {
    for (int i = 0; i < m_layers.size(); i++) {
        m_layers[i]->render(registry);
    }
}
void Level::update() {
    for (int i = 0; i < m_layers.size(); i++) {
        m_layers[i]->update(this);
    }
}