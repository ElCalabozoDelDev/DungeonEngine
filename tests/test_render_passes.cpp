#include <SDL.h>
#include <doctest/doctest.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/sprite_component.hpp>
#include <engine/components/texture_component.hpp>
#include <engine/components/tile_component.hpp>
#include <engine/components/tile_set_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/graphics/camera2d.hpp>
#include <engine/graphics/layer_passes.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <entt/entt.hpp>
#include <filesystem>
#include <memory>
#include <string>

using namespace de;

namespace
{
/// An 80x40 RGBA surface with a software renderer on it: real SDL drawing
/// with no window, so the passes can be checked pixel by pixel.
struct Canvas
{
    static constexpr int Width = 80;
    static constexpr int Height = 40;

    SDL_Surface* surface = nullptr;
    SDL_Renderer* renderer = nullptr;

    Canvas()
    {
        surface = SDL_CreateRGBSurfaceWithFormat(0, Width, Height, 32,
                                                 SDL_PIXELFORMAT_RGBA32);
        REQUIRE(surface != nullptr);
        renderer = SDL_CreateSoftwareRenderer(surface);
        REQUIRE(renderer != nullptr);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
    }

    ~Canvas()
    {
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(surface);
    }

    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

    /// Pixels in [x0, x1) x [y0, y1) that anything was drawn on.
    int drawnIn(int x0, int y0, int x1, int y1) const
    {
        const auto* pixels = static_cast<const Uint8*>(surface->pixels);
        int count = 0;
        for (int y = y0; y < y1; ++y)
        {
            for (int x = x0; x < x1; ++x)
            {
                const auto offset =
                    static_cast<std::size_t>(y * surface->pitch + x * 4);
                if (pixels[offset + 3] != 0)
                {
                    ++count;
                }
            }
        }
        return count;
    }
};

std::string asset(const char* relative)
{
    return (std::filesystem::path(DE_ASSETS_DIR) / relative).string();
}

/// A camera whose view is exactly the canvas, world origin at its top-left.
Camera2D canvasCamera()
{
    Camera2D camera;
    camera.position =
        Vector2D<float>(Canvas::Width * 0.5f, Canvas::Height * 0.5f);
    camera.viewWidth = Canvas::Width;
    camera.viewHeight = Canvas::Height;
    return camera;
}

void createLayer(entt::registry& registry, Layer layer)
{
    registry.ctx().get<SpatialIndex>().create(
        layer,
        [&registry](const entt::entity& entity)
        {
            const auto& p = registry.get<TransformComponent>(entity).position;
            const auto& d = registry.get<DimensionComponent>(entity);
            return Box<float>(p.getX(), p.getY(), d.width, d.height);
        },
        Box<float>(0.0f, 0.0f, 400.0f, 400.0f));
}

} // namespace

TEST_CASE(
    "SpriteLayerPass draws the sprites filed in its layer, where they are")
{
    Canvas canvas;
    entt::registry registry;
    registry.ctx().emplace<SpatialIndex>();
    auto& textures = registry.ctx().emplace<TextureCache>(canvas.renderer);
    REQUIRE(textures.load("slime", asset("images/slime.png")));
    createLayer(registry, Layer::Object);
    createLayer(registry, Layer::Overlay);

    const auto makeSprite = [&registry](float x, float y)
    {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<DimensionComponent>(entity, 20.0f, 20.0f);
        registry.emplace<TextureComponent>(entity, "slime");
        registry.emplace<SpriteComponent>(entity);
        return entity;
    };

    auto& index = registry.ctx().get<SpatialIndex>();
    REQUIRE(index.insert(Layer::Object, makeSprite(40.0f, 10.0f)));
    // In a different layer: this pass must not draw it.
    REQUIRE(index.insert(Layer::Overlay, makeSprite(0.0f, 10.0f)));

    SpriteLayerPass(Layer::Object, 20.0f).draw(registry, canvasCamera());

    CHECK(canvas.drawnIn(40, 10, 60, 30) > 0);
    CHECK(canvas.drawnIn(0, 0, 40, 40) == 0); // the Overlay sprite's place
    CHECK(canvas.drawnIn(60, 0, 80, 40) == 0);

    registry.ctx().erase<TextureCache>(); // before the renderer goes
}

TEST_CASE("TileLayerPass draws each tile's cell of its tileset")
{
    Canvas canvas;
    entt::registry registry;
    registry.ctx().emplace<SpatialIndex>();
    auto& textures = registry.ctx().emplace<TextureCache>(canvas.renderer);
    // Any 2x1 sheet of 20px cells serves as a tileset.
    REQUIRE(textures.load("sheet", asset("images/slime.png")));
    createLayer(registry, Layer::Bottom);

    auto tileset = registry.create();
    auto& set = registry.emplace<TileSetComponent>(tileset);
    set.firstGridID = 1;
    set.numColumns = 2;
    set.tileCount = 2;
    registry.emplace<DimensionComponent>(tileset, 20.0f, 20.0f);
    registry.emplace<TextureComponent>(tileset, "sheet");

    auto tile = registry.create();
    registry.emplace<TileComponent>(tile, 2, tileset);
    registry.emplace<TransformComponent>(tile, Vector2D<float>(20.0f, 20.0f));
    registry.emplace<DimensionComponent>(tile, 20.0f, 20.0f);
    REQUIRE(registry.ctx().get<SpatialIndex>().insert(Layer::Bottom, tile));

    TileLayerPass(Layer::Bottom, 20.0f).draw(registry, canvasCamera());

    CHECK(canvas.drawnIn(20, 20, 40, 40) > 0);
    CHECK(canvas.drawnIn(0, 0, 20, 40) == 0);
    CHECK(canvas.drawnIn(40, 0, 80, 40) == 0);

    registry.ctx().erase<TextureCache>();
}
