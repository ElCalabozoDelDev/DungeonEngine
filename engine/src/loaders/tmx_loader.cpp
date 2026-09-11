#include <SDL.h>
#include <base64.h>
#include <cstdint>
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
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <entt/entity/fwd.hpp>
#include <iostream>
#include <string>
#include <zlib.h>

namespace de
{
using namespace tinyxml2;

std::expected<void, std::string>
TMXLoader::loadLevel(entt::registry& registry,
                     const std::filesystem::path& levelFile)
{
    XMLDocument levelDocument;
    if (levelDocument.LoadFile(levelFile.string().c_str()) != XML_SUCCESS)
    {
        return std::unexpected("could not read level '" + levelFile.string() +
                               "': " + levelDocument.ErrorStr());
    }

    XMLElement* pRoot = levelDocument.RootElement();
    if (pRoot == nullptr)
    {
        return std::unexpected("level '" + levelFile.string() +
                               "' has no root element");
    }

    // Paths inside the file are relative to the file itself.
    m_levelDir = levelFile.parent_path();

    m_tilewidth = pRoot->IntAttribute("tilewidth");
    m_tileheight = pRoot->IntAttribute("tileheight");
    m_width = pRoot->IntAttribute("width");
    m_height = pRoot->IntAttribute("height");
    if (m_tilewidth <= 0 || m_tileheight <= 0 || m_width <= 0 || m_height <= 0)
    {
        return std::unexpected(
            "level '" + levelFile.string() +
            "' has a non-positive width/height/tilewidth/tileheight");
    }

    // Load the textures declared as map properties
    if (XMLElement* pProperties = pRoot->FirstChildElement("properties");
        pProperties != nullptr)
    {
        for (XMLElement* e = pProperties->FirstChildElement("property");
             e != nullptr; e = e->NextSiblingElement("property"))
        {
            loadTextures(registry, e);
        }
    }

    // Load the tilesets
    for (XMLElement* e = pRoot->FirstChildElement("tileset"); e != nullptr;
         e = e->NextSiblingElement("tileset"))
    {
        loadTilesets(registry, e);
    }

    // Load the object and tile layers
    for (XMLElement* e = pRoot->FirstChildElement(); e != nullptr;
         e = e->NextSiblingElement())
    {
        // Dispatch on the element name, not on which children it happens to
        // have. Guessing from the children meant a <layer> with no <data> --
        // a malformed file -- matched neither branch and was skipped without
        // a word.
        const std::string value = e->Value();
        if (value == "objectgroup")
        {
            // An object group with no objects is legitimate.
            loadObjectLayer(registry, e);
        }
        else if (value == "layer")
        {
            if (auto result = loadTileLayer(registry, e); !result)
            {
                return result;
            }
        }
    }

    auto levelEntity = registry.create();
    auto& levelComponent = registry.emplace<LevelComponent>(levelEntity);
    levelComponent.tilesets = m_tilesets;
    levelComponent.layers = m_layers;
    m_pEntities->push_back(levelEntity);

    return {};
}

void TMXLoader::loadTextures(entt::registry& registry, XMLElement* pTextureRoot)
{
    const char* value = pTextureRoot->Attribute("value");
    const char* name = pTextureRoot->Attribute("name");
    if (value == nullptr || name == nullptr)
    {
        return;
    }
    registry.ctx().get<TextureCache>().load(
        name, (m_levelDir / value).lexically_normal().string());
}

void TMXLoader::loadTilesets(entt::registry& registry, XMLElement* pTilesetRoot)
{
    const char* nameAttribute = pTilesetRoot->Attribute("name");
    XMLElement* pImage = pTilesetRoot->FirstChildElement("image");
    if (nameAttribute == nullptr || pImage == nullptr)
    {
        std::cerr << "TMXLoader: skipping a <tileset> with no name or image\n";
        return;
    }

    auto tileSetEntity = registry.create();
    auto& tileset = registry.emplace<TileSetComponent>(tileSetEntity);
    auto& dimension = registry.emplace<DimensionComponent>(tileSetEntity);
    registry.emplace<TextureComponent>(tileSetEntity, nameAttribute);

    const int imageWidth = pImage->IntAttribute("width");
    tileset.firstGridID = pTilesetRoot->IntAttribute("firstgid");
    dimension.width = pTilesetRoot->IntAttribute("tilewidth");
    dimension.height = pTilesetRoot->IntAttribute("tileheight");
    tileset.spacing = pTilesetRoot->IntAttribute("spacing");
    tileset.margin = pTilesetRoot->IntAttribute("margin");
    tileset.tileCount = pTilesetRoot->IntAttribute("tilecount");
    tileset.numColumns = dimension.width + tileset.spacing > 0
                             ? imageWidth / (dimension.width + tileset.spacing)
                             : 0;

    if (const char* source = pImage->Attribute("source"); source != nullptr)
    {
        registry.ctx().get<TextureCache>().load(
            nameAttribute, (m_levelDir / source).lexically_normal().string());
    }

    m_tilesets.push_back(tileSetEntity);
    m_pEntities->push_back(tileSetEntity);
}

entt::entity TMXLoader::tilesetFor(entt::registry& registry, int gid) const
{
    entt::entity best = entt::null;
    int bestFirstGid = 0;
    for (auto tilesetEntity : m_tilesets)
    {
        const auto& tileset = registry.get<TileSetComponent>(tilesetEntity);
        if (tileset.firstGridID <= gid && tileset.firstGridID >= bestFirstGid)
        {
            best = tilesetEntity;
            bestFirstGid = tileset.firstGridID;
        }
    }
    return best;
}

void TMXLoader::loadObjectLayer(entt::registry& registry,
                                XMLElement* pObjectElement)
{
    for (XMLElement* e = pObjectElement->FirstChildElement("object");
         e != nullptr; e = e->NextSiblingElement("object"))
    {
        auto entity = registry.create();

        int numFrames = 1;
        int spriteRow = 0;
        int spriteCol = 0;
        float animationTime = 0.0f;
        std::string textureID;

        const char* typeAttribute = e->Attribute("type");
        std::string type = typeAttribute != nullptr ? typeAttribute : "";

        const int x = e->IntAttribute("x");
        const int y = e->IntAttribute("y");
        const int width = e->IntAttribute("width");
        const int height = e->IntAttribute("height");

        if (XMLElement* properties = e->FirstChildElement("properties");
            properties != nullptr)
        {
            for (XMLElement* prop = properties->FirstChildElement("property");
                 prop != nullptr; prop = prop->NextSiblingElement("property"))
            {
                const char* name = prop->Attribute("name");
                const char* value = prop->Attribute("value");
                if (name == nullptr || value == nullptr)
                {
                    continue;
                }
                const std::string propertyName = name;
                if (propertyName == "totalFrames")
                {
                    numFrames = std::atoi(value);
                }
                else if (propertyName == "textureID")
                {
                    textureID = value;
                }
                else if (propertyName == "spriteRow")
                {
                    spriteRow = std::atoi(value);
                }
                else if (propertyName == "spriteCol")
                {
                    spriteCol = std::atoi(value);
                }
                else if (propertyName == "animationTime")
                {
                    animationTime = static_cast<float>(std::atof(value));
                }
            }
        }

        registry.emplace<TransformComponent>(
            entity,
            Vector2D<float>(static_cast<float>(x), static_cast<float>(y)));
        registry.emplace<TextureComponent>(entity, textureID);
        registry.emplace<DimensionComponent>(entity, width, height);
        registry.emplace<SpriteComponent>(entity, spriteRow, spriteCol, 0);
        registry.emplace<VelocityComponent>(entity, Vector2D<float>(0, 0));
        registry.emplace<AnimationComponent>(entity, spriteCol, numFrames,
                                             animationTime, 0);
        // Record the Tiled `type` verbatim and let the game decide what it
        // means; see InGameScene::tagObjectsByType.
        registry.emplace<ObjectTypeComponent>(entity, type);

        m_pEntities->push_back(entity);
        m_layers.push_back(entity);
    }
}

std::expected<void, std::string>
TMXLoader::loadTileLayer(entt::registry& registry, XMLElement* pTileElement)
{
    XMLElement* pDataNode = pTileElement->FirstChildElement("data");
    if (pDataNode == nullptr)
    {
        return std::unexpected("a <layer> has no <data> element");
    }

    std::string decodedIDs;
    for (XMLNode* e = pDataNode->FirstChild(); e != nullptr;
         e = e->NextSibling())
    {
        if (XMLText* text = e->ToText(); text != nullptr)
        {
            std::string raw = text->Value();
            decodedIDs = base64_decode(trim(raw));
        }
    }
    if (decodedIDs.empty())
    {
        return std::unexpected("a <layer> has an empty or unreadable <data> "
                               "payload (expected base64 + zlib)");
    }

    // One gid per tile. The old code sized the vector with the *byte* count,
    // allocating four times what it needed.
    std::vector<std::uint32_t> gids(static_cast<std::size_t>(m_width) *
                                    static_cast<std::size_t>(m_height));
    uLongf destinationBytes =
        static_cast<uLongf>(gids.size() * sizeof(std::uint32_t));

    const int zresult =
        uncompress(reinterpret_cast<Bytef*>(gids.data()), &destinationBytes,
                   reinterpret_cast<const Bytef*>(decodedIDs.data()),
                   static_cast<uLong>(decodedIDs.size()));
    if (zresult != Z_OK)
    {
        return std::unexpected(
            "could not zlib-decompress a <layer>: uncompress() returned " +
            std::to_string(zresult) +
            ". Tiled must export the layer with base64 + zlib compression.");
    }

    const char* nameAttribute = pTileElement->Attribute("name");
    const std::string name = nameAttribute != nullptr ? nameAttribute : "";

    auto layerEntity = registry.create();
    auto& tileLayer = registry.emplace<TileLayerComponent>(layerEntity);
    tileLayer.tileSetEntities = m_tilesets;

    if (name == "Bottom")
    {
        registry.emplace<BottomLayerComponent>(layerEntity);
    }
    else if (name == "Overlay")
    {
        registry.emplace<OverlayLayerComponent>(layerEntity);
    }
    else if (name == "Collision")
    {
        registry.emplace<CollisionLayerComponent>(layerEntity);
    }

    if (XMLElement* properties = pTileElement->FirstChildElement("properties");
        properties != nullptr)
    {
        for (XMLElement* prop = properties->FirstChildElement("property");
             prop != nullptr; prop = prop->NextSiblingElement("property"))
        {
            const char* propertyName = prop->Attribute("name");
            const char* value = prop->Attribute("value");
            if (propertyName != nullptr && value != nullptr &&
                std::string(propertyName) == "Collidable" &&
                std::string(value) == "true")
            {
                registry.emplace<CollisionComponent>(layerEntity);
            }
        }
    }

    bool reportedMissingTileset = false;
    for (int rows = 0; rows < m_height; rows++)
    {
        for (int cols = 0; cols < m_width; cols++)
        {
            const int tileId =
                static_cast<int>(gids[static_cast<std::size_t>(rows) *
                                          static_cast<std::size_t>(m_width) +
                                      static_cast<std::size_t>(cols)]);
            if (tileId == 0)
            {
                continue; // Skip empty tiles (tileId == 0)
            }

            const entt::entity tileset = tilesetFor(registry, tileId);
            if (tileset == entt::null)
            {
                if (!reportedMissingTileset)
                {
                    std::cerr << "TMXLoader: layer '" << name
                              << "' references tile " << tileId
                              << ", which no tileset covers; skipping it and "
                                 "any others like it\n";
                    reportedMissingTileset = true;
                }
                continue;
            }

            auto tileEntity = registry.create();
            const int tileX = cols * m_tilewidth;
            const int tileY = rows * m_tileheight;

            registry.emplace<TileComponent>(tileEntity, tileId, tileset);
            registry.emplace<TransformComponent>(
                tileEntity, Vector2D<float>(static_cast<float>(tileX),
                                            static_cast<float>(tileY)));
            registry.emplace<DimensionComponent>(tileEntity, m_tilewidth,
                                                 m_tileheight);

            tileLayer.tileEntities.push_back(tileEntity);
            m_pEntities->push_back(tileEntity);
        }
    }

    m_layers.push_back(layerEntity);
    m_pEntities->push_back(layerEntity);

    return {};
}

} // namespace de
