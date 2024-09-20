#include "loaders/entity_loader.hpp"
#include "tinyxml2.h"
#include <iostream>
#include <components/texture_component.hpp>
#include <components/position_component.hpp>
#include <components/velocity_component.hpp>
#include <components/player_component.hpp>
#include <components/animation_component.hpp>
#include <components/sprite_component.hpp>
#include <string>
#include "core/vector_2d.hpp"
#include "loaders/game_xml_path.hpp"
#include "core/texture_manager.hpp"

bool EntityLoader::loadPlayerDataFromXML(entt::registry &registry, SDL_Renderer *renderer)
{
    auto &gameXmlPath = registry.ctx().get<GameXmlPath>();
    auto &pRenderer = registry.ctx().get<SDL_Renderer *>();
    std::string path = gameXmlPath.path;
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << path << std::endl;
        return false;
    }
    tinyxml2::XMLElement *root = doc.FirstChildElement("Game")->FirstChildElement("Assets")->FirstChildElement("Player");
    if (root == nullptr)
    {
        std::cerr << "Failed to load player data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement *texture = root->FirstChildElement("Texture");
    if (texture == nullptr)
    {
        std::cerr << "Failed to load texture data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement *animation = root->FirstChildElement("Animation");
    if (animation == nullptr)
    {
        std::cerr << "Failed to load animation data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement *position = root->FirstChildElement("Position");
    if (position == nullptr)
    {
        std::cerr << "Failed to load position data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement *velocity = root->FirstChildElement("Speed");
    if (velocity == nullptr)
    {
        std::cerr << "Failed to load velocity data from XML" << std::endl;
        return false;
    }
    

    std::string texturePath = texture->Attribute("path");
    int spriteWidth = texture->IntAttribute("spriteWidth");
    int spriteHeight = texture->IntAttribute("spriteHeight");
    int spriteRow = texture->IntAttribute("spriteRow");
    int spriteCol = texture->IntAttribute("spriteCol");

    Vector2D positionVector(position->IntAttribute("x"), position->IntAttribute("y"));

    Vector2D velocityVector(velocity->IntAttribute("x"), velocity->IntAttribute("y"));


    int totalFrames = animation->IntAttribute("totalFrames");
    float animationTime = animation->FloatAttribute("animationTime");

    SDL_Texture *playerTexture;
    auto player = registry.create();
    std::string id = "player";
    if(TheTextureManager::Instance()->load(texturePath, id, pRenderer))
    {
        registry.emplace<PlayerComponent>(player);
        registry.emplace<TextureComponent>(player, id);
        registry.emplace<SpriteComponent>(player, spriteWidth, spriteHeight, spriteRow, spriteCol, 0);
        registry.emplace<PositionComponent>(player, positionVector);
        registry.emplace<VelocityComponent>(player, velocityVector);
        registry.emplace<AnimationComponent>(player, spriteCol, totalFrames, animationTime, 0);
        if (player == entt::null)
        {
            std::cerr << "Error al crear la entidad del jugador" << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error al cargar la textura del jugador" << std::endl;
        return false;
    }

    return true;
}