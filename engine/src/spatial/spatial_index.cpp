#include <engine/spatial/spatial_index.hpp>
#include <utility>

namespace de
{
void SpatialIndex::create(Layer layer, BoxFn getBox, const Box<float>& bounds)
{
    m_trees[indexOf(layer)].emplace(bounds, std::move(getBox));
}

SpatialIndex::Tree* SpatialIndex::get(Layer layer)
{
    auto& tree = m_trees[indexOf(layer)];
    return tree.has_value() ? &*tree : nullptr;
}

const SpatialIndex::Tree* SpatialIndex::get(Layer layer) const
{
    const auto& tree = m_trees[indexOf(layer)];
    return tree.has_value() ? &*tree : nullptr;
}

bool SpatialIndex::insert(Layer layer, entt::entity entity)
{
    auto& tree = m_trees[indexOf(layer)];
    return tree.has_value() && tree->add(entity);
}

bool SpatialIndex::remove(Layer layer, entt::entity entity)
{
    auto& tree = m_trees[indexOf(layer)];
    return tree.has_value() && tree->remove(entity);
}

std::vector<entt::entity> SpatialIndex::query(Layer layer,
                                              const Box<float>& box) const
{
    const auto& tree = m_trees[indexOf(layer)];
    if (!tree.has_value())
    {
        return {};
    }
    return tree->query(box);
}

void SpatialIndex::clear()
{
    for (auto& tree : m_trees)
    {
        tree.reset();
    }
}

} // namespace de
