#include <engine/spatial/spatial_index.hpp>
#include <utility>

namespace de
{
SpatialIndex::SpatialIndex()
{
    for (auto& data : m_layers)
    {
        data = std::make_unique<LayerData>();
    }
}

void SpatialIndex::create(Layer layer, BoxFn getBox, const Box<float>& bounds)
{
    auto& data = layerData(layer);
    data.filedAs.clear();
    data.getBox = std::move(getBox);
    // The tree navigates by the boxes this index remembers, never by the
    // entities' current bounds; see the class comment.
    const auto* filed = &data.filedAs;
    data.tree.emplace(bounds, [filed](const entt::entity& entity)
                      { return filed->at(entity); });
}

SpatialIndex::Tree* SpatialIndex::get(Layer layer)
{
    auto& tree = layerData(layer).tree;
    return tree.has_value() ? &*tree : nullptr;
}

const SpatialIndex::Tree* SpatialIndex::get(Layer layer) const
{
    const auto& tree = layerData(layer).tree;
    return tree.has_value() ? &*tree : nullptr;
}

bool SpatialIndex::insert(Layer layer, entt::entity entity)
{
    auto& data = layerData(layer);
    if (!data.tree.has_value() || !data.getBox || data.filedAs.contains(entity))
    {
        return false;
    }

    // Remembered before adding: the tree reads the box from filedAs.
    data.filedAs[entity] = data.getBox(entity);
    if (!data.tree->add(entity))
    {
        data.filedAs.erase(entity);
        return false;
    }
    return true;
}

bool SpatialIndex::remove(Layer layer, entt::entity entity)
{
    auto& data = layerData(layer);
    if (!data.tree.has_value())
    {
        return false;
    }

    auto it = data.filedAs.find(entity);
    if (it == data.filedAs.end())
    {
        return false;
    }

    const bool removed = data.tree->removeAt(entity, it->second);
    data.filedAs.erase(it);
    return removed;
}

bool SpatialIndex::refile(LayerData& data, entt::entity entity,
                          const Box<float>& current)
{
    auto it = data.filedAs.find(entity);
    data.tree->removeAt(entity, it->second);
    it->second = current;
    if (data.tree->add(entity))
    {
        return true;
    }
    // Moved outside the tree's bounds; stop tracking it rather than leaving
    // a box behind for something the tree does not hold.
    data.filedAs.erase(it);
    return false;
}

int SpatialIndex::update(entt::entity entity)
{
    int updated = 0;

    for (auto& layer : m_layers)
    {
        auto& data = *layer;
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

        refile(data, entity, current);
        ++updated;
    }

    return updated;
}

int SpatialIndex::updateLayer(const entt::registry& registry, Layer layer)
{
    auto& data = layerData(layer);
    if (!data.tree.has_value() || !data.getBox)
    {
        return 0;
    }

    // Collected first: re-filing edits the map being walked.
    std::vector<std::pair<entt::entity, Box<float>>> moved;
    std::vector<entt::entity> destroyed;
    for (const auto& [entity, filedBox] : data.filedAs)
    {
        if (!registry.valid(entity))
        {
            // Its box function would read components that no longer exist.
            destroyed.push_back(entity);
            continue;
        }
        const Box<float> current = data.getBox(entity);
        if (!(current == filedBox))
        {
            moved.emplace_back(entity, current);
        }
    }

    for (auto entity : destroyed)
    {
        remove(layer, entity);
    }
    for (const auto& [entity, current] : moved)
    {
        refile(data, entity, current);
    }
    return static_cast<int>(moved.size());
}

std::vector<entt::entity> SpatialIndex::query(Layer layer,
                                              const Box<float>& box) const
{
    const auto& tree = layerData(layer).tree;
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
        data->tree.reset();
        data->filedAs.clear();
        data->getBox = nullptr;
    }
}

} // namespace de
