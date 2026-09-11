#ifndef DE_PLUGINS_PLUGIN_HPP
#define DE_PLUGINS_PLUGIN_HPP

namespace de
{
class GameLoop;

class Plugin
{
public:
    virtual void mount(GameLoop& gameLoop) = 0;
    virtual ~Plugin() = default;
};

} // namespace de

#endif // DE_PLUGINS_PLUGIN_HPP
