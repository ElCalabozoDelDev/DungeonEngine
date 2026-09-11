#ifndef DE_WIDGETS_GUI_HPP
#define DE_WIDGETS_GUI_HPP
#include <type_traits>

#include <functional>
#include <optional>
#include <tuple>
#include <vector>

#include <entt/entt.hpp>

namespace de
{

// we want to restrain the callback type to std::function<R(Args...)>
// but since we already have a pack expansion with the dependencies,
// we need a C++ concept
namespace details
{
template <typename T>
struct is_callback_fn : std::false_type
{
};

template <typename R, typename... Args>
struct is_callback_fn<std::function<R(Args...)>> : std::true_type
{
};
}; // namespace details

namespace gui
{
template <typename T>
using setter_fn_type = std::function<void(const T&)>;

template <typename T>
using state_pair_type = std::pair<const T&, setter_fn_type<T>>;

using cleanup_fn_type = std::function<void()>;
using effect_fn_type = std::function<std::optional<cleanup_fn_type>()>;
template <typename T>
concept is_callback_fn = details::is_callback_fn<T>::value;

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

        // we get the current value
        const T& value = m_state.get<T>(key);
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
    template <typename... Deps>
    void use_effect(effect_fn_type effect_fn, std::tuple<Deps...> deps)
    {
        entt::id_type effect_key = m_current_index++;
        entt::id_type deps_key = m_current_index++;

        // if the effect does not exist yet
        if (!m_state.contains<std::tuple<Deps...>>(deps_key))
        {
            // we insert the current dependencies values in the state data
            m_state.emplace_as<std::tuple<Deps...>>(deps_key, deps);

            // we call the effect
            auto cleanup_fn = effect_fn();
            if (cleanup_fn)
            {
                // if the effect returned a cleanup function, we add it to the
                // state
                if (m_state.contains<cleanup_fn_type>(effect_key))
                {
                    m_state.erase<cleanup_fn_type>(effect_key);
                }

                m_state.emplace_as<cleanup_fn_type>(effect_key, *cleanup_fn);
            }
        }
        else
        {
            const auto& old_deps = m_state.get<std::tuple<Deps...>>(deps_key);

            // the effect exists, so we check if the dependencies changed
            if (deps != old_deps)
            {
                // if they did, we call the cleanup function (if it exists)
                if (m_state.contains<cleanup_fn_type>(effect_key))
                {
                    m_state.get<cleanup_fn_type>(effect_key)();
                    m_state.erase<cleanup_fn_type>(effect_key);
                }

                // we replace the dependencies
                m_state.erase<std::tuple<Deps...>>(deps_key);
                m_state.emplace_as<std::tuple<Deps...>>(deps_key, deps);

                // we call the effect
                auto cleanup_fn = effect_fn();
                if (cleanup_fn)
                {
                    // and set the cleanup function if needed
                    if (m_state.contains<cleanup_fn_type>(effect_key))
                    {
                        m_state.erase<cleanup_fn_type>(effect_key);
                    }

                    m_state.emplace_as<cleanup_fn_type>(effect_key,
                                                        *cleanup_fn);
                }
            }
        }
    }
    // here, the type parameter F must satisfy the concept we created earlier
    // this avoids the erroneous:
    // template <typename R, typename... Args, typename... Deps>
    template <is_callback_fn F, typename... Deps>
    F use_callback(F&& callback_fn, std::tuple<Deps...> deps)
    {
        entt::id_type cb_key = m_current_index++;
        entt::id_type deps_key = m_current_index++;

        // if the callback does not exists yet
        if (!m_state.contains<std::tuple<Deps...>>(deps_key))
        {
            // we insert the current dependencies into the state data
            m_state.emplace_as<std::tuple<Deps...>>(deps_key, deps);

            // and memoize the callback function
            if (m_state.contains<F>(cb_key))
            {
                m_state.erase<F>(cb_key);
            }

            m_state.emplace_as<F>(cb_key, callback_fn);
        }
        else
        {
            const auto& old_deps = m_state.get<std::tuple<Deps...>>(deps_key);

            // if the callback exists and the dependencies changed
            if (deps != old_deps)
            {
                // we replace the dependencies into the state data
                m_state.erase<std::tuple<Deps...>>(deps_key);
                m_state.emplace_as<std::tuple<Deps...>>(deps_key, deps);

                // we memoize the new callback function
                if (m_state.contains<F>(cb_key))
                {
                    m_state.erase<F>(cb_key);
                }

                m_state.emplace_as<F>(cb_key, callback_fn);
            }
        }

        // we return the memoized callback
        return m_state.get<F>(cb_key);
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
