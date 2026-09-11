#ifndef GAME_COMPONENTS_ENEMY_COMPONENT_HPP
#define GAME_COMPONENTS_ENEMY_COMPONENT_HPP

/// Marks a hostile entity. Chases the player and hurts on contact.
struct EnemyComponent
{
    float chaseRange = 120.0f;
    float speed = 60.0f;
    int contactDamage = 1;
};

#endif // GAME_COMPONENTS_ENEMY_COMPONENT_HPP
