#include "systems/collision_system.hpp"
#include "components/dimension_component.hpp"

#include "components/enemy_component.hpp"
#include "components/velocity_component.hpp"
#include "core/constants.hpp"

void CollisionSystem::run(entt::registry& registry)
{
    auto* objectQuadtree =
        QuadtreeManager::Instance()->getQuadtree(LayerType::OBJECT);

    auto* tileQuadtree =
        QuadtreeManager::Instance()->getQuadtree(LayerType::COLLISION);

    auto* overlayQuadtree =
        QuadtreeManager::Instance()->getQuadtree(LayerType::OVERLAY);

    if (!objectQuadtree || !tileQuadtree || !overlayQuadtree)
    {
        std::cerr << "Quadtrees no inicializados correctamente" << std::endl;
        return;
    }

    // Obtener todas las entidades de colisión del jugador
    auto playerEntities = objectQuadtree->query(objectQuadtree->getBox());

    // Iterar sobre cada entidad del jugador
    for (const auto& playerEntity : playerEntities)
    {
        // Obtener el Box de colisión del jugador
        auto& playerTransform = registry.get<TransformComponent>(playerEntity);
        auto& playerDimension = registry.get<DimensionComponent>(playerEntity);
        auto& playerVelocity = registry.get<VelocityComponent>(playerEntity);
        auto playerBox = Box<float>(
            playerTransform.position.getX(), playerTransform.position.getY(),
            playerDimension.width, playerDimension.height);
        // Consultar el Quadtree de los tiles para posibles colisiones
        auto potentialTiles = tileQuadtree->query(playerBox);

        // Verificar colisiones reales con los tiles
        for (const auto& tileEntity : potentialTiles)
        {
            auto tilePosition =
                registry.get<TransformComponent>(tileEntity).position;
            auto& tileDimension = registry.get<DimensionComponent>(tileEntity);
            auto tileBox =
                Box<float>(tilePosition.getX(), tilePosition.getY(),
                           tileDimension.width, tileDimension.height);
            if (playerBox.intersects(tileBox))
            {
                resolveCollision(playerVelocity, playerTransform, playerBox,
                                 tileBox);
            }
        }

        auto potentialOverlays = overlayQuadtree->query(playerBox);

        for (const auto& overlayEntity : potentialOverlays)
        {
            auto overlayPosition =
                registry.get<TransformComponent>(overlayEntity).position;
            auto& overlayDimension =
                registry.get<DimensionComponent>(overlayEntity);
            auto overlayBox =
                Box<float>(overlayPosition.getX(), overlayPosition.getY(),
                           overlayDimension.width, overlayDimension.height);
            if (playerBox.intersects(overlayBox))
            {
                resolveCollision(playerVelocity, playerTransform, playerBox,
                                 overlayBox);
            }
        }

        // Consultar colisiones con otras entidades en el mismo Quadtree
        auto potentialEntities = objectQuadtree->query(playerBox);
        for (const auto& entity : potentialEntities)
        {
            // Ignorar al jugador mismo
            if (entity == playerEntity)
                continue;

            // Verificar si es un enemigo
            if (registry.any_of<EnemyComponent>(entity))
            {
                auto& enemyTransform = registry.get<TransformComponent>(entity);
                auto& enemyDimension = registry.get<DimensionComponent>(entity);
                auto enemyBox =
                    Box<float>(enemyTransform.position.getX(),
                               enemyTransform.position.getY(),
                               enemyDimension.width, enemyDimension.height);

                // Si hay colisión con el enemigo, resolverla
                if (playerBox.intersects(enemyBox))
                {
                    resolvePlayerEnemyCollision(playerVelocity, playerTransform,
                                                playerBox, enemyBox);
                }
            }
        }
    }
}

void CollisionSystem::resolveCollision(VelocityComponent& velocity,
                                       TransformComponent& playerTransform,
                                       Box<float>& playerBox,
                                       const Box<float>& collidableBox)
{

    // Calcular la superposición en cada eje
    float overlapX = std::min(playerBox.getRight(), collidableBox.getRight()) -
                     std::max(playerBox.getLeft(), collidableBox.getLeft());

    float overlapY =
        std::min(playerBox.getBottom(), collidableBox.getBottom()) -
        std::max(playerBox.getTop(), collidableBox.getTop());

    // Resolver la colisión en el eje con la menor superposición
    if (overlapX < overlapY)
    {
        // Resolver la colisión en el eje X
        if (playerBox.getLeft() < collidableBox.getLeft())
        {
            playerTransform.position.setX(playerTransform.position.getX() -
                                          overlapX);
        }
        else
        {
            playerTransform.position.setX(playerTransform.position.getX() +
                                          overlapX);
        }
        velocity.velocity.setX(0);
    }
    else
    {
        // Resolver la colisión en el eje Y
        if (playerBox.getTop() < collidableBox.getTop())
        {
            playerTransform.position.setY(playerTransform.position.getY() -
                                          overlapY);
        }
        else
        {
            playerTransform.position.setY(playerTransform.position.getY() +
                                          overlapY);
        }
        velocity.velocity.setY(0);
    }

    // Actualizar el Box del jugador
    playerBox.setLeft(playerTransform.position.getX());
    playerBox.setTop(playerTransform.position.getY());
}

void CollisionSystem::resolvePlayerEnemyCollision(
    VelocityComponent& velocity, TransformComponent& playerTransform,
    Box<float>& playerBox, const Box<float>& enemyBox)
{
    // Calcular la superposición en cada eje
    float overlapX = std::min(playerBox.getRight(), enemyBox.getRight()) -
                     std::max(playerBox.getLeft(), enemyBox.getLeft());

    float overlapY = std::min(playerBox.getBottom(), enemyBox.getBottom()) -
                     std::max(playerBox.getTop(), enemyBox.getTop());

    // Resolver la colisión en el eje con menor superposición
    if (overlapX < overlapY)
    {
        // Resolver en el eje X
        if (playerBox.getLeft() < enemyBox.getLeft())
        {
            playerTransform.position.setX(playerTransform.position.getX() -
                                          overlapX);
        }
        else
        {
            playerTransform.position.setX(playerTransform.position.getX() +
                                          overlapX);
        }
        velocity.velocity.setX(0); // Detener la velocidad en el eje X
    }
    else
    {
        // Resolver en el eje Y
        if (playerBox.getTop() < enemyBox.getTop())
        {
            playerTransform.position.setY(playerTransform.position.getY() -
                                          overlapY);
        }
        else
        {
            playerTransform.position.setY(playerTransform.position.getY() +
                                          overlapY);
        }
        velocity.velocity.setY(0); // Detener la velocidad en el eje Y
    }

    // Aplicar daño o efectos al jugador tras la colisión
    std::cout << "El jugador ha colisionado con un enemigo, aplica el efecto "
                 "correspondiente."
              << std::endl;

    // Actualizar el Box de colisión del jugador
    playerBox.setLeft(playerTransform.position.getX());
    playerBox.setTop(playerTransform.position.getY());
}