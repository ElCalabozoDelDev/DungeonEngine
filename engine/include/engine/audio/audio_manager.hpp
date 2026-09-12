#ifndef DE_AUDIO_AUDIO_MANAGER_HPP
#define DE_AUDIO_AUDIO_MANAGER_HPP

#include <SDL_mixer.h>
#include <map>
#include <string>
#include <string_view>

namespace de
{
/// Owns the loaded sound effects and the mixer device.
///
/// Lives in the registry context and frees its chunks in the destructor, the
/// same ownership story as TextureCache. SDL2_mixer was linked from the start
/// and never used.
class AudioManager
{
public:
    AudioManager() = default;
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&& other) noexcept;
    AudioManager& operator=(AudioManager&& other) noexcept;

    /// Opens the mixer device. Returns an error message on failure; audio is
    /// not worth aborting startup over, so callers usually just report it and
    /// carry on muted.
    bool open(std::string* error = nullptr);

    bool isOpen() const { return m_open; }

    /// Loads a WAV under `id`, replacing anything already there.
    bool loadSound(std::string_view id, const std::string& fileName);

    /// Plays a previously loaded sound. Unknown ids and a closed device are
    /// no-ops.
    void playSound(std::string_view id, int volumePercent = 100) const;

    void clear();

private:
    void destroyAll() noexcept;

    bool m_open = false;
    std::map<std::string, Mix_Chunk*, std::less<>> m_sounds;
};

} // namespace de

#endif // DE_AUDIO_AUDIO_MANAGER_HPP
