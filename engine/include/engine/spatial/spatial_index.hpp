#ifndef DE_SPATIAL_SPATIAL_INDEX_HPP
#define DE_SPATIAL_SPATIAL_INDEX_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <engine/spatial/quadtree.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <optional>
#include <vector>

namespace de
{
/// The render/collision layers a level is partitioned into.
///
/// An enum rather than the string keys the old QuadtreeManager used: the
/// render passes look these up several times per frame, and every lookup was
/// building a std::string and walking a red-black tree.
enum class Layer : std::uint8_t
{
    Bottom,
    Overlay,
    Collision,
    Object,
    Count
};

/// Per-layer quadtrees over the entities of the current level.
///
/// Lives in the registry context, so it dies with the registry instead of
/// outliving the level like the QuadtreeManager singleton did. Changing scenes
/// must call clear(); otherwise the trees keep handles to destroyed entities
/// and the next query hands them back.
class SpatialIndex
{
public:
    using BoxFn = std::function<Box<float>(const entt::entity&)>;
    using Tree = Quadtree<entt::entity, BoxFn>;

    SpatialIndex() = default;

    SpatialIndex(const SpatialIndex&) = delete;
    SpatialIndex& operator=(const SpatialIndex&) = delete;
    SpatialIndex(SpatialIndex&&) = default;
    SpatialIndex& operator=(SpatialIndex&&) = default;

    /// Builds the tree for `layer`, replacing any tree already there.
    void create(Layer layer, BoxFn getBox, const Box<float>& bounds);

    /// Returns nullptr when `layer` has no tree yet.
    Tree* get(Layer layer);
    const Tree* get(Layer layer) const;

    /// False when the layer has no tree, or the entity's box falls outside it.
    bool insert(Layer layer, entt::entity entity);

    /// False when the layer has no tree, or the entity was not in it.
    bool remove(Layer layer, entt::entity entity);

    std::vector<entt::entity> query(Layer layer, const Box<float>& box) const;

    /// Drops every tree. Call this whenever the entities they refer to are
    /// about to be destroyed.
    void clear();

private:
    static constexpr std::size_t LayerCount =
        static_cast<std::size_t>(Layer::Count);

    static constexpr std::size_t indexOf(Layer layer)
    {
        return static_cast<std::size_t>(layer);
    }

    std::array<std::optional<Tree>, LayerCount> m_trees;
};

} // namespace de

#endif // DE_SPATIAL_SPATIAL_INDEX_HPP
