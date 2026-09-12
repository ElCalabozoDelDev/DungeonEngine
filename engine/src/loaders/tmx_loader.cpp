#include <SDL.h>
#include <cctype>
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
#include <engine/core/vector_2d.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/loaders/tmx_loader.hpp>
#include <entt/entity/fwd.hpp>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace de
{
using namespace tinyxml2;

namespace
{
/// Parses Tiled's CSV tile payload into exactly `expectedCount` gids.
/// Empty tokens (trailing commas, blank lines) are skipped.
std::expected<std::vector<std::uint32_t>, std::string>
parseCsvGids(const std::string& csv, std::size_t expectedCount)
{
    std::vector<std::uint32_t> gids;
    gids.reserve(expectedCount);

    std::size_t start = 0;
    while (start <= csv.size())
    {
        const std::size_t comma = csv.find(',', start);
        const std::string_view raw =
            comma == std::string::npos
                ? std::string_view(csv).substr(start)
                : std::string_view(csv).substr(start, comma - start);

        std::size_t begin = 0;
        while (begin < raw.size() &&
               std::isspace(static_cast<unsigned char>(raw[begin])) != 0)
        {
            ++begin;
        }
        std::size_t end = raw.size();
        while (end > begin &&
               std::isspace(static_cast<unsigned char>(raw[end - 1])) != 0)
        {
            --end;
        }

        if (begin < end)
        {
            const std::string token(raw.substr(begin, end - begin));
            try
            {
                const unsigned long value = std::stoul(token);
                gids.push_back(static_cast<std::uint32_t>(value));
            }
            catch (const std::exception&)
            {
                return std::unexpected(
                    "a <layer> has a non-integer CSV tile id: '" + token + "'");
            }
        }

        if (comma == std::string::npos)
        {
            break;
        }
        start = comma + 1;
    }

    if (gids.size() != expectedCount)
    {
        return std::unexpected(
            "a <layer> CSV payload has " + std::to_string(gids.size()) +
            " tile ids; expected " + std::to_string(expectedCount) +
            " (width * height)");
    }

    return gids;
}
} // namespace

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
        // currentFrame starts at 0, not at spriteCol. It is an offset from
        // SpriteComponent::currentCol, so seeding it with the column made the
        // first drawn frame 2 * spriteCol -- past the end of the run for any
        // object that did not start at column 0.
        AnimationComponent animation;
        animation.currentFrame = 0;
        animation.totalFrames = numFrames;
        animation.animationTime = animationTime;
        registry.emplace<AnimationComponent>(entity, animation);
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

    const char* encodingAttr = pDataNode->Attribute("encoding");
    const std::string encoding = encodingAttr != nullptr ? encodingAttr : "";
    if (encoding != "csv")
    {
        return std::unexpected("a <layer> uses encoding '" + encoding +
                               "'; expected encoding=\"csv\"");
    }

    std::string csvText;
    for (XMLNode* e = pDataNode->FirstChild(); e != nullptr;
         e = e->NextSibling())
    {
        if (XMLText* text = e->ToText(); text != nullptr)
        {
            csvText += text->Value();
        }
    }

    const std::size_t expectedCount =
        static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
    auto gidsResult = parseCsvGids(csvText, expectedCount);
    if (!gidsResult)
    {
        return std::unexpected(gidsResult.error());
    }
    const std::vector<std::uint32_t>& gids = *gidsResult;

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
