#ifndef DE_INPUT_INPUT_STATE_HPP
#define DE_INPUT_INPUT_STATE_HPP

#include <SDL.h>
#include <algorithm>
#include <array>
#include <cstddef>

namespace de
{
/// Keyboard and mouse state for the current frame, published in the registry
/// context by InputPlugin.
///
/// Systems read this instead of calling SDL_GetKeyboardState themselves. That
/// gives them edge detection (held vs just-pressed), one consistent snapshot
/// for every system in the frame -- including repeated fixed steps -- and a
/// single place that knows whether Dear ImGui has the keyboard.
class InputState
{
public:
    /// True while the key is held. Always false when a GUI widget has focus.
    bool isDown(SDL_Scancode key) const
    {
        return !m_keyboardCaptured && get(m_current, key);
    }

    /// True only on the frame the key went down.
    bool wasPressed(SDL_Scancode key) const
    {
        return !m_keyboardCaptured && wasPressedRaw(key);
    }

    /// Like wasPressed, but ignores ImGui keyboard capture. Menu widgets use
    /// this so an open overlay cannot swallow Enter / arrows.
    bool wasPressedRaw(SDL_Scancode key) const
    {
        return get(m_current, key) && !get(m_previous, key);
    }

    /// True only on the frame the key came up.
    bool wasReleased(SDL_Scancode key) const
    {
        return !m_keyboardCaptured && !get(m_current, key) &&
               get(m_previous, key);
    }

    int mouseX() const { return m_mouseX; }
    int mouseY() const { return m_mouseY; }

    bool isMouseDown(int button) const
    {
        return !m_mouseCaptured && (m_mouseButtons & SDL_BUTTON(button)) != 0;
    }

    /// True when Dear ImGui is using the keyboard, so typing in a text field
    /// does not also drive the game.
    bool keyboardCaptured() const { return m_keyboardCaptured; }
    bool mouseCaptured() const { return m_mouseCaptured; }

    // --- Written by InputPlugin ---

    void beginFrame() { m_previous = m_current; }

    void setKeyboard(const Uint8* state, int count)
    {
        if (state == nullptr)
        {
            return;
        }
        const std::size_t n =
            std::min(m_current.size(), static_cast<std::size_t>(count));
        std::copy_n(state, n, m_current.begin());
    }

    void setMouse(int x, int y, Uint32 buttons)
    {
        m_mouseX = x;
        m_mouseY = y;
        m_mouseButtons = buttons;
    }

    void setCaptured(bool keyboard, bool mouse)
    {
        m_keyboardCaptured = keyboard;
        m_mouseCaptured = mouse;
    }

private:
    using KeyArray = std::array<Uint8, SDL_NUM_SCANCODES>;

    static bool get(const KeyArray& keys, SDL_Scancode key)
    {
        const auto index = static_cast<std::size_t>(key);
        return index < keys.size() && keys[index] != 0;
    }

    KeyArray m_current{};
    KeyArray m_previous{};
    int m_mouseX = 0;
    int m_mouseY = 0;
    Uint32 m_mouseButtons = 0;
    bool m_keyboardCaptured = false;
    bool m_mouseCaptured = false;
};

} // namespace de

#endif // DE_INPUT_INPUT_STATE_HPP
