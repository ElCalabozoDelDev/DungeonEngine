#include "scene/entity_loader.hpp"
#include "SDL_image.h"
#include "tinyxml2.h"
#include <iostream>
#include <components/texture_component.hpp>
#include <components/position_component.hpp>
#include <components/velocity_component.hpp>
#include <components/player_component.hpp>
#include <components/animation_component.hpp>

bool EntityLoader::loadPlayerDataFromXML(const std::string &path, entt::registry &registry, SDL_Renderer *renderer)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << path << std::endl;
        return false;
    }
    tinyxml2::XMLElement *root = doc.FirstChildElement("Assets")->FirstChildElement("Player");
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

    int x = position->IntAttribute("x");
    int y = position->IntAttribute("y");
    
    int vx = velocity->IntAttribute("vx");
    int vy = velocity->IntAttribute("vy");

    int totalFrames = animation->IntAttribute("totalFrames");
    float animationTime = animation->FloatAttribute("animationTime");

    SDL_Texture *playerTexture = loadTexture(texturePath, renderer);
    auto player = registry.create();
    registry.emplace<TextureComponent>(player, playerTexture, spriteWidth, spriteHeight, spriteRow, spriteCol);
    registry.emplace<PositionComponent>(player, x, y);
    registry.emplace<VelocityComponent>(player, vx, vy);
    registry.emplace<PlayerComponent>(player);
    registry.emplace<AnimationComponent>(player, 0, spriteCol, totalFrames, animationTime, 0);
    if (player == entt::null)
    {
        std::cerr << "Error al crear la entidad del jugador" << std::endl;
        return false;
    }
    return true;
}

SDL_Texture *EntityLoader::loadTexture(const std::string &path,
                                       SDL_Renderer *renderer)
{
    SDL_Texture *newTexture = IMG_LoadTexture(renderer, path.c_str());
    if (newTexture == nullptr)
    {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
    }
    return newTexture;
}