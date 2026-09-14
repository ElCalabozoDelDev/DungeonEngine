#ifndef DE_WIDGETS_GUI_HPP
#define DE_WIDGETS_GUI_HPP

#include <functional>
#include <utility>
#include <vector>

#include <entt/entt.hpp>

namespace de
{
namespace gui
{
template <typename T>
using setter_fn_type = std::function<void(const T&)>;

/// The value is a copy, not a reference into the hook storage: the next
/// use_state() in the same frame may emplace and rehash that storage, which
/// left an earlier reference dangling on a widget's first frame. Setters are
/// deferred to commit(), so a copy never goes stale within the frame.
template <typename T>
using state_pair_type = std::pair<T, setter_fn_type<T>>;

class Hooks
{
private:
    using mutation_type = std::function<void()>;

    entt::id_type m_current_index{0};
    entt::registry::context m_state;
    std::vector<mutation_type> m_mutation_queue;

public:
    Hooks() : m_state(std::allocator<entt::entity>()) {}
    void reset() { m_current_index = 0; }

    void commit()
    {
        for (const auto& mutation : m_mutation_queue)
        {
            mutation();
        }

        m_mutation_queue.clear();
    }
    template <typename T>
    state_pair_type<T> use_state(const T& initial_value)
    {
        entt::id_type key = m_current_index++;

        // if the value does not exist yet, we create it
        if (!m_state.contains<T>(key))
        {
            m_state.emplace_as<T>(key, initial_value);
        }

        // we copy the current value (see state_pair_type)
        T value = m_state.get<T>(key);
        // and generate a setter function
        auto update_fn = [this, key](const T& new_value) -> void
        {
            // the setter function simply push the mutation to the queue
            m_mutation_queue.push_back(
                [this, key, new_value]() -> void
                {
                    // we replace the value in the state data
                    m_state.erase<T>(key);
                    m_state.emplace_as<T>(key, new_value);
                });
        };

        // we return the pair
        return {value, update_fn};
    }
};

class WidgetComponent
{
private:
    Hooks m_hooks;

public:
    WidgetComponent() = default;
    virtual ~WidgetComponent() = default;
    virtual void render(entt::registry& registry, Hooks& h) = 0;

    void frame_begin() { m_hooks.reset(); }

    void frame_update(entt::registry& registry) { render(registry, m_hooks); }

    void frame_end() { m_hooks.commit(); }
};

} // namespace gui

} // namespace de

#endif // DE_WIDGETS_GUI_HPP
