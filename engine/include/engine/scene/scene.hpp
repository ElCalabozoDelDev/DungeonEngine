#ifndef DE_SCENE_SCENE_HPP
#define DE_SCENE_SCENE_HPP

#include <entt/entt.hpp>
#include <span>
#include <vector>

namespace de
{
/// One screen of the game: a title, a level, an options menu.
///
/// A scene owns the entities it creates. Handing them to track() is enough:
/// SceneSystem destroys every tracked entity right after onExit(), so a scene
/// only needs onExit() for clean-up that is not an entity it made.
class Scene
{
public:
    virtual ~Scene() = default;

    virtual void onEnter(entt::registry& registry) = 0;

    /// Every frame while the scene is active. Most scenes keep their
    /// behaviour in systems and widgets and leave this alone.
    virtual void onUpdate(entt::registry& /*registry*/) {}

    /// Runs before the tracked entities are destroyed.
    virtual void onExit(entt::registry& /*registry*/) {}

    /// Destroys every tracked entity that still exists. Called by SceneSystem
    /// after onExit(); scenes do not call it themselves.
    void destroyTracked(entt::registry& registry);

protected:
    /// Marks `entity` as the scene's, to be destroyed when it exits.
    entt::entity track(entt::entity entity);
    void track(std::span<const entt::entity> entities);

private:
    std::vector<entt::entity> m_tracked;
};

} // namespace de

#endif // DE_SCENE_SCENE_HPP
