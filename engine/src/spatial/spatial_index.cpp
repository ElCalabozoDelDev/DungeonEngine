#include <engine/spatial/spatial_index.hpp>
#include <utility>

namespace de
{
void SpatialIndex::create(Layer layer, BoxFn getBox, const Box<float>& bounds)
{
    auto& data = m_layers[indexOf(layer)];
    data.filedAs.clear();
    data.getBox = getBox;
    data.tree.emplace(bounds, std::move(getBox));
}

SpatialIndex::Tree* SpatialIndex::get(Layer layer)
{
    auto& tree = m_layers[indexOf(layer)].tree;
    return tree.has_value() ? &*tree : nullptr;
}

const SpatialIndex::Tree* SpatialIndex::get(Layer layer) const
{
    const auto& tree = m_layers[indexOf(layer)].tree;
    return tree.has_value() ? &*tree : nullptr;
}

bool SpatialIndex::insert(Layer layer, entt::entity entity)
{
    auto& data = m_layers[indexOf(layer)];
    if (!data.tree.has_value() || !data.getBox)
    {
        return false;
    }

    const Box<float> box = data.getBox(entity);
    if (!data.tree->add(entity))
    {
        return false;
    }
    data.filedAs[entity] = box;
    return true;
}

bool SpatialIndex::remove(Layer layer, entt::entity entity)
{
    auto& data = m_layers[indexOf(layer)];
    if (!data.tree.has_value())
    {
        return false;
    }

    auto it = data.filedAs.find(entity);
    if (it == data.filedAs.end())
    {
        return false;
    }

    // Remove with the box it was filed under, not its current one.
    const bool removed = data.tree->removeAt(entity, it->second);
    data.filedAs.erase(it);
    return removed;
}

int SpatialIndex::update(entt::entity entity)
{
    int updated = 0;

    for (auto& data : m_layers)
    {
        if (!data.tree.has_value() || !data.getBox)
        {
            continue;
        }

        auto it = data.filedAs.find(entity);
        if (it == data.filedAs.end())
        {
            continue; // this layer does not hold the entity
        }

        const Box<float> current = data.getBox(entity);
        if (current == it->second)
        {
            continue; // has not moved
        }

        data.tree->removeAt(entity, it->second);
        if (data.tree->add(entity))
        {
            it->second = current;
        }
        else
        {
            // Moved outside the tree's bounds; stop tracking it rather than
            // leaving a stale box behind.
            data.filedAs.erase(it);
        }
        ++updated;
    }

    return updated;
}

std::vector<entt::entity> SpatialIndex::query(Layer layer,
                                              const Box<float>& box) const
{
    const auto& tree = m_layers[indexOf(layer)].tree;
    if (!tree.has_value())
    {
        return {};
    }
    return tree->query(box);
}

void SpatialIndex::clear()
{
    for (auto& data : m_layers)
    {
        data.tree.reset();
        data.filedAs.clear();
        data.getBox = nullptr;
    }
}

} // namespace de
