#include <SDL.h>
#include <iostream>
#include "world/level_parser.hpp"
#include "base64.h"
#include "components/animation_component.hpp"
#include "components/level_component.hpp"
#include "components/player_component.hpp"
#include "components/transform_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "components/velocity_component.hpp"
#include "components/tile_component.hpp"
#include "core/texture_manager.hpp"
#include "core/vector_2d.hpp"
#include "entt/entity/fwd.hpp"
#include "tinyxml2.h"
#include "zlib.h"
#include "components/tile_set_component.hpp"
#include "components/tile_layer_component.hpp"
#include "core/trim.hpp"

// using namespace tinyxml2;

void LevelParser::parseLevel(entt::registry &registry,
                               const char *levelFile) {
  // create a TinyXML document and load the map XML
  XMLDocument levelDocument;
  levelDocument.LoadFile(levelFile);
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
      parseTilesets(registry, e);
    }
  }
  // parse any object layers
  for (XMLElement *e = pRoot->FirstChildElement(); e != NULL;
       e = e->NextSiblingElement()) {
    if (e->Value() == std::string("objectgroup") || e->Value() == std::string("layer")) {
      if (e->FirstChildElement()->Value() == std::string("object"))
      {
        parseObjectLayer(registry, e);
      }
      else if (e->FirstChildElement()->Value() == std::string("data")||
                    (e->FirstChildElement()->NextSiblingElement() != 0 && e->FirstChildElement()->NextSiblingElement()->Value() == std::string("data")))
      {
        parseTileLayer(registry, e);
      }
    }
  }
  auto levelEntity = registry.create();
  auto & levelComponent = registry.emplace<LevelComponent>(levelEntity);
  levelComponent.tilesets = m_tilesets;
  levelComponent.layers = m_layers;
  m_pEntities->push_back(levelEntity);
}

void LevelParser::parseTextures(entt::registry &registry,
                                XMLElement *pTextureRoot) {
  // load the textures
  std::string path = pTextureRoot->Attribute("value");
  std::string id = pTextureRoot->Attribute("name");
  TheTextureManager::Instance()->load(path, id, registry.ctx().get<SDL_Renderer *>());
}

void LevelParser::parseTilesets(entt::registry &registry,
                                XMLElement *pTilesetRoot) {

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
  m_tilesets.push_back(tileSetEntity);
  m_pEntities->push_back(tileSetEntity);
}

void LevelParser::parseObjectLayer(entt::registry &registry,
                                   XMLElement *pObjectElement) {
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
      registry.emplace<TransformComponent>(entity, Vector2D(x, y));
      registry.emplace<TextureComponent>(entity, textureID);

      registry.emplace<SpriteComponent>(entity, width, height, spriteRow, spriteCol, 0);
      registry.emplace<VelocityComponent>(entity, Vector2D(0, 0));
      registry.emplace<AnimationComponent>(entity, spriteCol, numFrames, animationTime, 0);
      if (type == "Player")
      {
        registry.emplace<PlayerComponent>(entity);
      }
      m_pEntities->push_back(entity);
      m_layers.push_back(entity);
    }
  }
} 

void LevelParser::parseTileLayer(entt::registry &registry,
                                 XMLElement *pTileElement) {
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
  auto layerEntity = registry.create();
  auto & tileLayer = registry.emplace<TileLayerComponent>(layerEntity);
  tileLayer.tileSize = m_tileSize;
  tileLayer.numColumns = m_width;
  tileLayer.numRows = m_height;
  tileLayer.mapWidth = m_width * m_tileSize;
  tileLayer.mapHeight = m_height * m_tileSize;
  tileLayer.tileSetEntities = m_tilesets;

  registry.emplace<TransformComponent>(layerEntity, Vector2D(0, 0));

  // std::vector<int> layerRow(m_width);
  // for (int j = 0; j < m_height; j++) {
  //   data.push_back(layerRow);
  // }
  for (int rows = 0; rows < m_height; rows++) {
    for (int cols = 0; cols < m_width; cols++) {
      int tileId = gids[rows * m_width + cols];
      if (tileId == 0) {
        continue; // Ignorar tiles vacíos (tileId == 0)
      }
      auto tileEntity = registry.create();
      int tileX = cols * m_tileSize;
      int tileY = rows * m_tileSize;

      registry.emplace<TransformComponent>(tileEntity, Vector2D(tileX, tileY));
      auto &tile = registry.emplace<TileComponent>(tileEntity);
      tile.tileId = tileId;
      tile.tileX = cols;
      tile.tileY = rows;

      tileLayer.tileEntities.push_back(tileEntity);
      m_pEntities->push_back(tileEntity);
    }
  }
  m_layers.push_back(layerEntity);
  m_pEntities->push_back(layerEntity);
}