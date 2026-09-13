#include <SDL.h>
#include <algorithm>
#include <engine/audio/audio_manager.hpp>
#include <iostream>
#include <utility>

namespace de
{
int AudioManager::clampPercent(int percent)
{
    return std::clamp(percent, 0, 100);
}

AudioManager::~AudioManager() { destroyAll(); }

AudioManager::AudioManager(AudioManager&& other) noexcept
    : m_open(other.m_open), m_sfxVolume(other.m_sfxVolume),
      m_musicVolume(other.m_musicVolume), m_sounds(std::move(other.m_sounds)),
      m_music(std::move(other.m_music))
{
    other.m_sounds.clear();
    other.m_music.clear();
    other.m_open = false;
}

AudioManager& AudioManager::operator=(AudioManager&& other) noexcept
{
    if (this != &other)
    {
        destroyAll();
        m_open = other.m_open;
        m_sfxVolume = other.m_sfxVolume;
        m_musicVolume = other.m_musicVolume;
        m_sounds = std::move(other.m_sounds);
        m_music = std::move(other.m_music);
        other.m_sounds.clear();
        other.m_music.clear();
        other.m_open = false;
    }
    return *this;
}

void AudioManager::destroyAll() noexcept
{
    stopMusic();

    for (auto& [id, chunk] : m_sounds)
    {
        if (chunk != nullptr)
        {
            Mix_FreeChunk(chunk);
        }
    }
    m_sounds.clear();

    for (auto& [id, music] : m_music)
    {
        if (music != nullptr)
        {
            Mix_FreeMusic(music);
        }
    }
    m_music.clear();

    if (m_open)
    {
        Mix_CloseAudio();
        Mix_Quit();
        m_open = false;
    }
}

bool AudioManager::open(std::string* error)
{
    if (m_open)
    {
        return true;
    }

    const int flags = MIX_INIT_OGG;
    if ((Mix_Init(flags) & flags) != flags)
    {
        // Still try to open: WAV SFX may work without OGG.
        std::cerr << "AudioManager: Mix_Init: " << Mix_GetError() << '\n';
    }

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        if (error != nullptr)
        {
            *error = Mix_GetError();
        }
        Mix_Quit();
        return false;
    }
    m_open = true;
    setSfxVolume(m_sfxVolume);
    setMusicVolume(m_musicVolume);
    return true;
}

bool AudioManager::loadSound(std::string_view id, const std::string& fileName)
{
    if (!m_open)
    {
        return false;
    }

    Mix_Chunk* chunk = Mix_LoadWAV(fileName.c_str());
    if (chunk == nullptr)
    {
        std::cerr << "AudioManager: could not load '" << fileName
                  << "': " << Mix_GetError() << '\n';
        return false;
    }

    auto key = std::string(id);
    if (auto it = m_sounds.find(key); it != m_sounds.end())
    {
        Mix_FreeChunk(it->second);
        it->second = chunk;
        return true;
    }
    m_sounds.emplace(std::move(key), chunk);
    return true;
}

bool AudioManager::loadMusic(std::string_view id, const std::string& fileName)
{
    if (!m_open)
    {
        return false;
    }

    Mix_Music* music = Mix_LoadMUS(fileName.c_str());
    if (music == nullptr)
    {
        std::cerr << "AudioManager: could not load music '" << fileName
                  << "': " << Mix_GetError() << '\n';
        return false;
    }

    auto key = std::string(id);
    if (auto it = m_music.find(key); it != m_music.end())
    {
        Mix_FreeMusic(it->second);
        it->second = music;
        return true;
    }
    m_music.emplace(std::move(key), music);
    return true;
}

void AudioManager::playSound(std::string_view id) const
{
    if (!m_open)
    {
        return;
    }
    auto it = m_sounds.find(id);
    if (it == m_sounds.end() || it->second == nullptr)
    {
        return;
    }
    Mix_VolumeChunk(it->second, MIX_MAX_VOLUME * m_sfxVolume / 100);
    Mix_PlayChannel(-1, it->second, 0);
}

void AudioManager::playMusic(std::string_view id, bool loop)
{
    if (!m_open)
    {
        return;
    }
    auto it = m_music.find(id);
    if (it == m_music.end() || it->second == nullptr)
    {
        return;
    }
    Mix_VolumeMusic(MIX_MAX_VOLUME * m_musicVolume / 100);
    Mix_PlayMusic(it->second, loop ? -1 : 0);
}

void AudioManager::stopMusic()
{
    if (m_open)
    {
        Mix_HaltMusic();
    }
}

void AudioManager::setSfxVolume(int percent)
{
    m_sfxVolume = clampPercent(percent);
}

void AudioManager::setMusicVolume(int percent)
{
    m_musicVolume = clampPercent(percent);
    if (m_open)
    {
        Mix_VolumeMusic(MIX_MAX_VOLUME * m_musicVolume / 100);
    }
}

void AudioManager::clear() { destroyAll(); }

} // namespace de
