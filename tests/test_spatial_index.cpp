#include <algorithm>
#include <doctest/doctest.h>
#include <engine/spatial/spatial_index.hpp>
#include <entt/entt.hpp>
#include <vector>

using namespace de;

namespace
{
struct Pos
{
    float x;
    float y;
};

SpatialIndex::BoxFn boxFnFor(entt::registry& registry)
{
    return [&registry](const entt::entity& entity)
    {
        const auto& p = registry.get<Pos>(entity);
        return Box<float>(p.x, p.y, 8.0f, 8.0f);
    };
}

bool findable(SpatialIndex& index, entt::registry& registry,
              entt::entity entity)
{
    const auto& p = registry.get<Pos>(entity);
    const auto hits =
        index.query(Layer::Object, Box<float>(p.x - 2, p.y - 2, 12, 12));
    return std::find(hits.begin(), hits.end(), entity) != hits.end();
}

} // namespace

TEST_CASE("a moved entity stays findable once the index is updated")
{
    // Regression test. An entity is filed under the box it had when it was
    // inserted; once it moves, queries near where it actually is never
    // descend into the node that still holds it.
    //
    // 40 entities, because the split threshold is 16: with fewer, the tree
    // never branches and its root leaf checks every value's *current* box,
    // which hides the bug entirely.
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry),
                 Box<float>(0, 0, 1000, 1000));

    std::vector<entt::entity> all;
    for (int i = 0; i < 40; ++i)
    {
        auto e = registry.create();
        registry.emplace<Pos>(e, static_cast<float>((i % 8) * 100 + 10),
                              static_cast<float>((i / 8) * 100 + 10));
        REQUIRE(index.insert(Layer::Object, e));
        all.push_back(e);
    }

    const entt::entity mover = all.front();
    REQUIRE(findable(index, registry, mover));

    // Move it into a different branch of the tree.
    registry.get<Pos>(mover) = Pos{900.0f, 900.0f};

    CHECK_FALSE(findable(index, registry, mover)); // the bug, without update
    CHECK(index.update(mover) == 1);
    CHECK(findable(index, registry, mover)); // the fix

    SUBCASE("and is no longer reported where it used to be")
    {
        const auto stale = index.query(Layer::Object, Box<float>(8, 8, 12, 12));
        CHECK(std::find(stale.begin(), stale.end(), mover) == stale.end());
    }
}

TEST_CASE("update is a no-op for an entity that did not move")
{
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry), Box<float>(0, 0, 100, 100));

    auto e = registry.create();
    registry.emplace<Pos>(e, 10.0f, 10.0f);
    REQUIRE(index.insert(Layer::Object, e));

    CHECK(index.update(e) == 0);
}

TEST_CASE("remove takes an entity out of the index")
{
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry), Box<float>(0, 0, 100, 100));

    auto e = registry.create();
    registry.emplace<Pos>(e, 10.0f, 10.0f);
    REQUIRE(index.insert(Layer::Object, e));

    CHECK(index.remove(Layer::Object, e));
    CHECK_FALSE(findable(index, registry, e));
    CHECK_FALSE(index.remove(Layer::Object, e)); // already gone
}

TEST_CASE("insert reports failure instead of asserting")
{
    // add()/remove() used to rely on asserts, which vanish under NDEBUG and
    // left the tree silently corrupt in release builds.
    entt::registry registry;
    SpatialIndex index;

    auto e = registry.create();
    registry.emplace<Pos>(e, 5000.0f, 5000.0f);

    CHECK_FALSE(index.insert(Layer::Object, e)); // no tree for that layer yet

    index.create(Layer::Object, boxFnFor(registry), Box<float>(0, 0, 100, 100));
    CHECK_FALSE(index.insert(Layer::Object, e)); // outside the bounds
}

TEST_CASE("clear drops every layer")
{
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry), Box<float>(0, 0, 100, 100));

    auto e = registry.create();
    registry.emplace<Pos>(e, 10.0f, 10.0f);
    REQUIRE(index.insert(Layer::Object, e));

    index.clear();
    CHECK(index.get(Layer::Object) == nullptr);
    CHECK(index.query(Layer::Object, Box<float>(0, 0, 100, 100)).empty());
}

TEST_CASE("updateLayer re-files entities that moved without a velocity")
{
    // Regression test. SpatialSyncSystem only re-filed entities with a
    // VelocityComponent, so the bat and the snake -- which move themselves --
    // stayed filed wherever they were inserted.
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry),
                 Box<float>(0, 0, 1000, 1000));

    std::vector<entt::entity> all;
    for (int i = 0; i < 40; ++i)
    {
        auto e = registry.create();
        registry.emplace<Pos>(e, static_cast<float>((i % 8) * 100 + 10),
                              static_cast<float>((i / 8) * 100 + 10));
        REQUIRE(index.insert(Layer::Object, e));
        all.push_back(e);
    }

    const entt::entity mover = all.front();
    registry.get<Pos>(mover) = Pos{900.0f, 900.0f};
    REQUIRE_FALSE(findable(index, registry, mover));

    CHECK(index.updateLayer(registry, Layer::Object) == 1);
    CHECK(findable(index, registry, mover));
    CHECK(index.updateLayer(registry, Layer::Object) == 0); // nothing moved
}

TEST_CASE("updateLayer drops entities that were destroyed")
{
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry), Box<float>(0, 0, 100, 100));

    auto e = registry.create();
    registry.emplace<Pos>(e, 10.0f, 10.0f);
    REQUIRE(index.insert(Layer::Object, e));

    // Destroyed without being removed: the box function would read a Pos
    // that no longer exists.
    registry.destroy(e);
    CHECK(index.updateLayer(registry, Layer::Object) == 0);
    CHECK(index.query(Layer::Object, Box<float>(0, 0, 100, 100)).empty());
    CHECK_FALSE(index.remove(Layer::Object, e)); // no longer tracked
}

TEST_CASE("updateLayer stays consistent when many entities move at once")
{
    // Regression test. When a re-filed entity makes a node split, the tree
    // distributes that node's values by their boxes. If it asks for their
    // *current* boxes, an entity that has moved but has not been re-filed yet
    // lands in a branch its remembered box does not lead to: removing it
    // then fails silently and adding it again leaves a duplicate. The snake
    // moves every segment in the same step, so this is the normal case.
    entt::registry registry;
    SpatialIndex index;
    index.create(Layer::Object, boxFnFor(registry),
                 Box<float>(0, 0, 1000, 1000));

    std::vector<entt::entity> all;
    for (int i = 0; i < 64; ++i)
    {
        auto e = registry.create();
        registry.emplace<Pos>(e, static_cast<float>((i % 8) * 120 + 10),
                              static_cast<float>((i / 8) * 120 + 10));
        REQUIRE(index.insert(Layer::Object, e));
        all.push_back(e);
    }

    // Everything moves in the same step: mirror the grid, so every entity
    // changes quadrant and the nodes empty and refill as it is re-filed.
    for (int round = 0; round < 3; ++round)
    {
        for (auto e : all)
        {
            auto& p = registry.get<Pos>(e);
            p = Pos{980.0f - p.x, 980.0f - p.y};
        }
        index.updateLayer(registry, Layer::Object);

        auto everything =
            index.query(Layer::Object, Box<float>(0, 0, 1000, 1000));
        std::sort(everything.begin(), everything.end());
        CHECK(std::adjacent_find(everything.begin(), everything.end()) ==
              everything.end()); // no duplicates
        CHECK(everything.size() == all.size());
        for (auto e : all)
        {
            CHECK(findable(index, registry, e));
        }
    }
}
