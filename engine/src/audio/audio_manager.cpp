#include <SDL.h>
#include <engine/audio/audio_manager.hpp>
#include <iostream>
#include <utility>

namespace de
{
AudioManager::~AudioManager() { destroyAll(); }

AudioManager::AudioManager(AudioManager&& other) noexcept
    : m_open(other.m_open), m_sounds(std::move(other.m_sounds))
{
    other.m_sounds.clear();
    other.m_open = false;
}

AudioManager& AudioManager::operator=(AudioManager&& other) noexcept
{
    if (this != &other)
    {
        destroyAll();
        m_open = other.m_open;
        m_sounds = std::move(other.m_sounds);
        other.m_sounds.clear();
        other.m_open = false;
    }
    return *this;
}

void AudioManager::destroyAll() noexcept
{
    for (auto& [id, chunk] : m_sounds)
    {
        if (chunk != nullptr)
        {
            Mix_FreeChunk(chunk);
        }
    }
    m_sounds.clear();

    if (m_open)
    {
        Mix_CloseAudio();
        m_open = false;
    }
}

bool AudioManager::open(std::string* error)
{
    if (m_open)
    {
        return true;
    }
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
    {
        if (error != nullptr)
        {
            *error = Mix_GetError();
        }
        return false;
    }
    m_open = true;
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

void AudioManager::playSound(std::string_view id, int volumePercent) const
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
    Mix_VolumeChunk(it->second, MIX_MAX_VOLUME * volumePercent / 100);
    Mix_PlayChannel(-1, it->second, 0);
}

void AudioManager::clear() { destroyAll(); }

} // namespace de
