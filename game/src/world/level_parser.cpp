#include <SDL.h>
#include <iostream>
#include "world/level_parser.hpp"
#include "base64.h"
#include "components/animation_component.hpp"
#include "components/player_component.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "components/velocity_component.hpp"
#include "core/texture_manager.hpp"
#include "core/vector_2d.hpp"
#include "world/level.hpp"
#include "entt/entity/fwd.hpp"
#include "tinyxml2.h"
#include "zlib.h"
#include "components/tile_set_component.hpp"
#include "components/tile_layer_component.hpp"
#include "core/trim.hpp"

// using namespace tinyxml2;

Level *LevelParser::parseLevel(entt::registry &registry,
                               const char *levelFile) {
  // create a TinyXML document and load the map XML
  XMLDocument levelDocument;
  levelDocument.LoadFile(levelFile);
  // create the level object
  Level *pLevel = new Level();
  // get the root node
  XMLElement *pRoot = levelDocument.RootElement();
  m_tileSize = pRoot->IntAttribute("tilewidth");
  m_width = pRoot->IntAttribute("width");
  m_height = pRoot->IntAttribute("height");

  XMLElement *pProperties = pRoot->FirstChildElement();
  // parse the textures
  for (XMLElement *e = pProperties->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("property")) {
      parseTextures(registry, e);
    }
  }
  // parse the tilesets
  for (XMLElement *e = pRoot->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("tileset")) {
      parseTilesets(registry, e, pLevel->getTilesets());
    }
  }
  // parse any object layers
  for (XMLElement *e = pRoot->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("objectgroup") || e->Value() == std::string("layer")) {
      if (e->FirstChildElement()->Value() == std::string("object"))
      {
        parseObjectLayer(registry, e, pLevel->getLayers(), pLevel);
      }
      else if (e->FirstChildElement()->Value() == std::string("data")||
                    (e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::string("data")))
      {
        parseTileLayer(registry, e, pLevel->getLayers(), pLevel->getTilesets());
      }
    }
  }
  return pLevel;
}

void LevelParser::parseTextures(entt::registry &registry,
                                XMLElement *pTextureRoot) {
  // load the textures
  std::string path = pTextureRoot->Attribute("value");
  std::string id = pTextureRoot->Attribute("name");
  TheTextureManager::Instance()->load(path, id, registry.ctx().get<SDL_Renderer *>());
}

void LevelParser::parseTilesets(entt::registry &registry,
                                XMLElement *pTilesetRoot,
                                std::vector<entt::entity> *pTilesets) {

  auto *pRenderer = registry.ctx().get<SDL_Renderer *>();
  std::string assetsTag = "../assets/Levels/";
  // Obtener el atributo "name"
  const char* nameAttribute = pTilesetRoot->Attribute("name");
  // first add the tileset to texture manager
  // create a tileset object
  auto tileSetEntity = registry.create();
  auto &tileset = registry.emplace<TileSetComponent>(tileSetEntity);
  registry.emplace<TextureComponent>(tileSetEntity, nameAttribute);
  tileset.width = pTilesetRoot->FirstChildElement()->IntAttribute("width");
  tileset.height = pTilesetRoot->FirstChildElement()->IntAttribute("height");
  tileset.firstGridID = pTilesetRoot->IntAttribute("firstgid");
  tileset.tileWidth = pTilesetRoot->IntAttribute("tilewidth");
  tileset.tileHeight = pTilesetRoot->IntAttribute("tileheight");
  tileset.spacing = pTilesetRoot->IntAttribute("spacing");
  tileset.margin = pTilesetRoot->IntAttribute("margin");
  tileset.tileCount = pTilesetRoot->IntAttribute("tilecount");
  tileset.numColumns = tileset.width / (tileset.tileWidth + tileset.spacing);
  TheTextureManager::Instance()->load(assetsTag.append(pTilesetRoot->FirstChildElement()->Attribute("source")), nameAttribute, pRenderer);
  pTilesets->push_back(tileSetEntity);
}

void LevelParser::parseObjectLayer(entt::registry &registry,
                                   XMLElement *pObjectElement,
                                   std::vector<entt::entity> *pLayers,
                                   Level *pLevel) {
  for (XMLElement *e = pObjectElement->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("object")) {
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
      for (XMLElement *properties = e->FirstChildElement(); properties != NULL;
           properties = properties->NextSiblingElement()) {
        if (properties->Value() == std::string("properties")) {
          for (XMLElement *prop = properties->FirstChildElement();
               prop != NULL; prop = prop->NextSiblingElement()) {
            if (prop->Value() == std::string("property")) {
              std::string name = prop->Attribute("name");
              std::string value = prop->Attribute("value");
              if (name == "totalFrames") {
                numFrames = atoi(value.c_str());
              } else if (name == "textureID") {
                textureID = value;
              } else if (name == "spriteRow") {
                spriteRow = atoi(value.c_str());
              } else if (name == "spriteCol") {
                spriteCol = atoi(value.c_str());
              } else if (name == "animationTime") {
                animationTime = atof(value.c_str());
              }
            }
          }
        }
      }
      // add the object to the object list
      auto &pos = registry.emplace<PositionComponent>(entity, Vector2D(x, y));
      registry.emplace<TextureComponent>(entity, textureID);

      registry.emplace<SpriteComponent>(entity, width, height, spriteRow, spriteCol, 0);
      registry.emplace<VelocityComponent>(entity, Vector2D(0, 0));
      registry.emplace<AnimationComponent>(entity, spriteCol, numFrames, animationTime, 0);
      if (type == "Player")
      {
        registry.emplace<PlayerComponent>(entity);
      }
      pLayers->push_back(entity);
    }
  }
} 

void LevelParser::parseTileLayer(entt::registry &registry,
                                 XMLElement *pTileElement,
                                 std::vector<entt::entity> *pLayers,
                                 const std::vector<entt::entity> *pTilesets) {
  // tile data
  std::vector<std::vector<int>> data;
  std::string decodedIDs;
  XMLElement *pDataNode;
  for (XMLElement *e = pTileElement->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("data")) {
      pDataNode = e;
    }
  }
  for (XMLNode *e = pDataNode->FirstChild(); e != NULL; e = e->NextSibling()) {
    XMLText *text = e->ToText();
    std::string t = text->Value();
    decodedIDs = base64_decode(trim(t));
  }
  // uncompress zlib compression
  uLongf numGids = m_width * m_height * sizeof(int);
  std::vector<unsigned> gids(numGids);
  uncompress((Bytef *)&gids[0], &numGids, (const Bytef *)decodedIDs.c_str(),
             decodedIDs.size());
  std::vector<int> layerRow(m_width);
  for (int j = 0; j < m_height; j++) {
    data.push_back(layerRow);
  }
  for (int rows = 0; rows < m_height; rows++) {
    for (int cols = 0; cols < m_width; cols++) {
      data[rows][cols] = gids[rows * m_width + cols];
    }
  }
  auto layerEntity = registry.create();
  registry.emplace<TileLayerComponent>(layerEntity, m_tileSize, pTilesets, data, m_width, m_height, m_width);
  registry.emplace<PositionComponent>(layerEntity, Vector2D(0, 0));
  pLayers->push_back(layerEntity);
}