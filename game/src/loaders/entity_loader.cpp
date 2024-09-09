#include "loaders/entity_loader.hpp"
#include "SDL_image.h"
#include "tinyxml2.h"
#include <SDL_surface.h>
#include <iostream>
#include <components/texture_component.hpp>
#include <components/position_component.hpp>
#include <components/velocity_component.hpp>
#include <components/player_component.hpp>
#include <components/animation_component.hpp>
#include "core/vector_2d.hpp"
#include "loaders/game_xml_path.hpp"

bool EntityLoader::loadPlayerDataFromXML(entt::registry &registry, SDL_Renderer *renderer)
{
    auto &gameXmlPath = registry.ctx().get<GameXmlPath>();
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

    SDL_Texture *playerTexture = loadTexture(texturePath, renderer);
    auto player = registry.create();
    registry.emplace<TextureComponent>(player, playerTexture, spriteWidth, spriteHeight, spriteRow, spriteCol);
    registry.emplace<PositionComponent>(player, positionVector);
    registry.emplace<VelocityComponent>(player, velocityVector);
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
    SDL_Surface *tempSurface = IMG_Load(path.c_str());
    if (tempSurface == nullptr)
    {
        std::cerr << "Failed to load surface: " << IMG_GetError() << std::endl;
    }
    SDL_Texture *newTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    if (newTexture == nullptr)
    {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
    }
    return newTexture;
}