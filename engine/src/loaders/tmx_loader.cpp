#include "base64.h"
#include "entt/entity/fwd.hpp"
#include "tinyxml2.h"
#include "zlib.h"
#include <SDL.h>
#include <engine/components/animation_component.hpp>
#include <engine/components/bottom_layer_component.hpp>
#include <engine/components/collision_component.hpp>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/level_component.hpp>
#include <engine/components/object_type_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/trim.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/sdl_resources.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <engine/spatial/quadtree.hpp>
#include <iostream>
#include <string>

namespace de
{
using namespace tinyxml2;

void TMXLoader::loadLevel(entt::registry& registry, const char* levelFile)
{
    // create a TinyXML document and load the map XML
    XMLDocument levelDocument;
    levelDocument.LoadFile(levelFile);
    // get the root node
    XMLElement* pRoot = levelDocument.RootElement();
    m_tilewidth = pRoot->IntAttribute("tilewidth");
    m_tileheight = pRoot->IntAttribute("tileheight");
    m_width = pRoot->IntAttribute("width");
    m_height = pRoot->IntAttribute("height");

    XMLElement* pProperties = pRoot->FirstChildElement();
    // load the textures
    for (XMLElement* e = pProperties->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("property"))
        {
            loadTextures(registry, e);
        }
    }
    // load the tilesets
    for (XMLElement* e = pRoot->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("tileset"))
        {
            loadTilesets(registry, e);
        }
    }
    // load any object layers
    for (XMLElement* e = pRoot->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("objectgroup") ||
            e->Value() == std::string("layer"))
        {
            if (e->FirstChildElement()->Value() == std::string("object"))
            {
                loadObjectLayer(registry, e);
            }
            else if (e->FirstChildElement()->Value() == std::string("data") ||
                     (e->FirstChildElement()->NextSiblingElement() != 0 &&
                      e->FirstChildElement()->NextSiblingElement()->Value() ==
                          std::string("data")))
            {
                loadTileLayer(registry, e);
            }
        }
    }
    auto levelEntity = registry.create();
    auto& levelComponent = registry.emplace<LevelComponent>(levelEntity);
    levelComponent.tilesets = m_tilesets;
    levelComponent.layers = m_layers;
    m_pEntities->push_back(levelEntity);
}

void TMXLoader::loadTextures(entt::registry& registry, XMLElement* pTextureRoot)
{
    // load the textures
    std::string path = pTextureRoot->Attribute("value");
    std::string id = pTextureRoot->Attribute("name");
    registry.ctx().get<TextureCache>().load(id, path);
}

void TMXLoader::loadTilesets(entt::registry& registry, XMLElement* pTilesetRoot)
{

    std::string assetsTag = "../assets/Levels/";
    // The tileset "name" attribute
    const char* nameAttribute = pTilesetRoot->Attribute("name");
    // create a tileset object
    auto tileSetEntity = registry.create();
    auto& tileset = registry.emplace<TileSetComponent>(tileSetEntity);
    auto& dimension = registry.emplace<DimensionComponent>(tileSetEntity);
    registry.emplace<TextureComponent>(tileSetEntity, nameAttribute);
    int width = pTilesetRoot->FirstChildElement()->IntAttribute("width");
    tileset.firstGridID = pTilesetRoot->IntAttribute("firstgid");
    dimension.width = pTilesetRoot->IntAttribute("tilewidth");
    dimension.height = pTilesetRoot->IntAttribute("tileheight");
    tileset.spacing = pTilesetRoot->IntAttribute("spacing");
    tileset.margin = pTilesetRoot->IntAttribute("margin");
    tileset.tileCount = pTilesetRoot->IntAttribute("tilecount");
    tileset.numColumns = width / (dimension.width + tileset.spacing);
    registry.ctx().get<TextureCache>().load(
        nameAttribute,
        assetsTag.append(
            pTilesetRoot->FirstChildElement()->Attribute("source")));
    m_tilesets.push_back(tileSetEntity);
    m_pEntities->push_back(tileSetEntity);
}

void TMXLoader::loadObjectLayer(entt::registry& registry,
                                XMLElement* pObjectElement)
{
    for (XMLElement* e = pObjectElement->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("object"))
        {
            auto entity = registry.create();
            int x, y, width, height, numFrames, spriteRow = 0, spriteCol = 0;
            float animationTime = 0;
            std::string textureID;
            std::string type = e->Attribute("type");
            // get the initial node values
            e->QueryIntAttribute("x", &x);
            e->QueryIntAttribute("y", &y);

            width = e->IntAttribute("width");
            height = e->IntAttribute("height");

            // get the property values
            for (XMLElement* properties = e->FirstChildElement();
                 properties != NULL;
                 properties = properties->NextSiblingElement())
            {
                if (properties->Value() == std::string("properties"))
                {
                    for (XMLElement* prop = properties->FirstChildElement();
                         prop != NULL; prop = prop->NextSiblingElement())
                    {
                        if (prop->Value() == std::string("property"))
                        {
                            std::string name = prop->Attribute("name");
                            std::string value = prop->Attribute("value");
                            if (name == "totalFrames")
                            {
                                numFrames = atoi(value.c_str());
                            }
                            else if (name == "textureID")
                            {
                                textureID = value;
                            }
                            else if (name == "spriteRow")
                            {
                                spriteRow = atoi(value.c_str());
                            }
                            else if (name == "spriteCol")
                            {
                                spriteCol = atoi(value.c_str());
                            }
                            else if (name == "animationTime")
                            {
                                animationTime = atof(value.c_str());
                            }
                        }
                    }
                }
            }
            // add the object to the object list
            registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
            registry.emplace<TextureComponent>(entity, textureID);
            registry.emplace<DimensionComponent>(entity, width, height);
            registry.emplace<SpriteComponent>(entity, spriteRow, spriteCol, 0);
            registry.emplace<VelocityComponent>(entity, Vector2D<float>(0, 0));
            registry.emplace<AnimationComponent>(entity, spriteCol, numFrames,
                                                 animationTime, 0);
            // Record the Tiled `type` verbatim and let the game decide
            // what it means; see InGameScene::tagObjectsByType.
            registry.emplace<ObjectTypeComponent>(entity, type);
            m_pEntities->push_back(entity);
            m_layers.push_back(entity);
        }
    }
}

void TMXLoader::loadTileLayer(entt::registry& registry,
                              XMLElement* pTileElement)
{
    std::string decodedIDs;
    XMLElement* pDataNode;
    for (XMLElement* e = pTileElement->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("data"))
        {
            pDataNode = e;
        }
    }
    for (XMLNode* e = pDataNode->FirstChild(); e != NULL; e = e->NextSibling())
    {
        XMLText* text = e->ToText();
        std::string t = text->Value();
        decodedIDs = base64_decode(trim(t));
    }
    // uncompress zlib compression
    uLongf numGids = m_width * m_height * sizeof(int);
    std::vector<unsigned> gids(numGids);
    uncompress((Bytef*)&gids[0], &numGids, (const Bytef*)decodedIDs.c_str(),
               decodedIDs.size());
    std::string name = pTileElement->Attribute("name");
    auto layerEntity = registry.create();
    auto& tileLayer = registry.emplace<TileLayerComponent>(layerEntity);
    tileLayer.tileSetEntities = m_tilesets;

    if (name == "Bottom")
    {
        registry.emplace<BottomLayerComponent>(layerEntity);
    }
    if (name == "Overlay")
    {
        registry.emplace<OverlayLayerComponent>(layerEntity);
    }
    else if (name == "Collision")
    {
        registry.emplace<CollisionLayerComponent>(layerEntity);
    }

    for (XMLElement* e = pTileElement->FirstChildElement(); e != NULL;
         e = e->NextSiblingElement())
    {
        if (e->Value() == std::string("properties"))
        {
            for (XMLElement* prop = e->FirstChildElement(); prop != NULL;
                 prop = prop->NextSiblingElement())
            {
                if (prop->Value() == std::string("property"))
                {
                    std::string name = prop->Attribute("name");
                    std::string value = prop->Attribute("value");
                    if (name == "Collidable")
                    {
                        if (value == "true")
                        {
                            registry.emplace<CollisionComponent>(layerEntity);
                        }
                    }
                }
            }
        }
    }

    for (int rows = 0; rows < m_height; rows++)
    {
        for (int cols = 0; cols < m_width; cols++)
        {
            int tileId = gids[rows * m_width + cols];
            if (tileId == 0)
            {
                continue; // Skip empty tiles (tileId == 0)
            }
            auto tileEntity = registry.create();
            int tileX = cols * m_tilewidth;
            int tileY = rows * m_tileheight;

            auto& tile = registry.emplace<TileComponent>(tileEntity);
            registry.emplace<TransformComponent>(tileEntity,
                                                 Vector2D<float>(tileX, tileY));
            registry.emplace<DimensionComponent>(tileEntity, m_tilewidth,
                                                 m_tileheight);
            tile.tileId = tileId;

            tileLayer.tileEntities.push_back(tileEntity);
            m_pEntities->push_back(tileEntity);
        }
    }

    m_layers.push_back(layerEntity);
    m_pEntities->push_back(layerEntity);
}

} // namespace de
