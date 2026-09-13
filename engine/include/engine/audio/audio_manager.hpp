#ifndef DE_AUDIO_AUDIO_MANAGER_HPP
#define DE_AUDIO_AUDIO_MANAGER_HPP

#include <SDL_mixer.h>
#include <map>
#include <string>
#include <string_view>

namespace de
{
/// Owns loaded sound effects, optional music, and the mixer device.
///
/// Lives in the registry context and frees its chunks / music in the
/// destructor, the same ownership story as TextureCache.
class AudioManager
{
public:
    AudioManager() = default;
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    AudioManager(AudioManager&& other) noexcept;
    AudioManager& operator=(AudioManager&& other) noexcept;

    /// Opens the mixer device. Returns false on failure; audio is not worth
    /// aborting startup over, so callers usually report it and carry on muted.
    bool open(std::string* error = nullptr);

    bool isOpen() const { return m_open; }

    /// Loads a WAV (or other Mix_LoadWAV format) under `id`.
    bool loadSound(std::string_view id, const std::string& fileName);

    /// Loads streaming music (e.g. OGG) under `id`.
    bool loadMusic(std::string_view id, const std::string& fileName);

    /// Plays a previously loaded sound using the current SFX volume.
    void playSound(std::string_view id) const;

    /// Plays previously loaded music. `loop` true means infinite loop.
    /// No-op if `id` is already the track that is playing.
    void playMusic(std::string_view id, bool loop = true);

    void stopMusic();

    /// True while Mix_PlayingMusic reports active playback.
    bool isMusicPlaying() const;

    /// Id of the track last started with playMusic, or empty.
    std::string_view currentMusicId() const { return m_currentMusicId; }

    /// 0–100. Applied to subsequent SFX plays and Mix_VolumeMusic.
    void setSfxVolume(int percent);
    void setMusicVolume(int percent);

    int sfxVolume() const { return m_sfxVolume; }
    int musicVolume() const { return m_musicVolume; }

    void clear();

private:
    void destroyAll() noexcept;
    static int clampPercent(int percent);

    bool m_open = false;
    int m_sfxVolume = 100;
    int m_musicVolume = 100;
    std::string m_currentMusicId;
    std::map<std::string, Mix_Chunk*, std::less<>> m_sounds;
    std::map<std::string, Mix_Music*, std::less<>> m_music;
};

} // namespace de

#endif // DE_AUDIO_AUDIO_MANAGER_HPP
