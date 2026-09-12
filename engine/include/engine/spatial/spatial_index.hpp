#ifndef DE_SPATIAL_SPATIAL_INDEX_HPP
#define DE_SPATIAL_SPATIAL_INDEX_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <engine/spatial/quadtree.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <optional>
#include <unordered_map>
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
/// Alongside each tree it keeps the box every entity was filed under. A
/// quadtree locates a value by its bounds, so once an entity moves, its new
/// bounds lead down a different branch and the tree can neither find nor
/// remove it -- and queries near its new position never descend into the node
/// that still holds it. update() removes with the remembered box and re-adds
/// with the current one.
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

    /// Re-files `entity` under its current bounds, in whichever layers hold
    /// it. Entities whose bounds have not changed cost one comparison.
    /// Returns the number of layers that were actually re-filed.
    int update(entt::entity entity);

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

    struct LayerData
    {
        std::optional<Tree> tree;
        /// Kept alongside the tree so update() can recompute current bounds.
        BoxFn getBox;
        /// The box each entity was filed under, needed to remove it again
        /// after it has moved.
        std::unordered_map<entt::entity, Box<float>> filedAs;
    };

    std::array<LayerData, LayerCount> m_layers;
};

} // namespace de

#endif // DE_SPATIAL_SPATIAL_INDEX_HPP
