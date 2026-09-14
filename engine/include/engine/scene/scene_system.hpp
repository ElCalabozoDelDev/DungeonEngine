#ifndef DE_SCENE_SCENE_SYSTEM_HPP
#define DE_SCENE_SCENE_SYSTEM_HPP

#include <engine/scene/scene.hpp>
#include <engine/systems/system.hpp>
#include <memory>

namespace de
{
/// Owns the active scene and switches between them.
///
/// The loop owns this system; the registry context holds a reference to it
/// (`ctx().get<SceneSystem&>()`) rather than a second shared_ptr, so there is
/// one owner rather than two.
class SceneSystem : public System
{
public:
    void run(entt::registry& registry) override;

    /// Switches immediately. Only safe outside the frame -- use it from a
    /// setup callback to install the first scene.
    void setScene(entt::registry& registry, std::unique_ptr<Scene> newScene);

    /// Asks for a switch, applied at the start of the next run().
    ///
    /// Switching destroys every entity the old scene created. Doing that in
    /// the middle of a frame pulls the ground out from under the systems that
    /// have not run yet -- and from under whichever system asked for the
    /// switch, which is usually iterating a view at the time.
    void requestScene(std::unique_ptr<Scene> newScene);

private:
    std::unique_ptr<Scene> m_currentScene;
    std::unique_ptr<Scene> m_pendingScene;
};

} // namespace de

#endif // DE_SCENE_SCENE_SYSTEM_HPP
