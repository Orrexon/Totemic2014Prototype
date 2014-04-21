#include "AudioSystem.h"
#include <iostream>
#include "SFML\Audio\Sound.hpp"

namespace
{
	int clamp(int number, int min, int max)
	{
		if (number > max) return max;
		if (number < min) return min;
		return number;
	}
	float clampf(float number, float min, float max)
	{
		if (number > max) return max;
		if (number < min) return min;
		return number;
	}
}

Audiosystem::Audiosystem(float masterVolume /*= 1.f*/, int soundVolume /*= 100*/, int musicVolume /*= 100*/)
{
	m_masterVolume = masterVolume;
	m_soundVolume = soundVolume;
	m_musicVolume = musicVolume;
}

Audiosystem::~Audiosystem()
{
	auto soundIt = m_sounds.begin();
	while (soundIt != m_sounds.end())
	{
		delete soundIt->second;
		soundIt->second = nullptr;
		++soundIt;
	}
	m_sounds.clear();


	auto musicIt = m_music.begin();
	while (musicIt != m_music.end())
	{
		delete musicIt->second;
		musicIt->second = nullptr;
		++musicIt;
	}
	m_music.clear();
}

void Audiosystem::createSound(std::string identifier, std::string filepath)
{
	auto it = m_sounds.find(identifier);
	if (it != m_sounds.end())
	{
		std::cout << "Failed to create sound. Sound '" << filepath << "' does already exist." << std::endl;
		return;
	}

	float myModifiedVolume = static_cast<float>(m_soundVolume)* m_masterVolume;

	Sound* sound = new Sound();
	sound->setVolume(myModifiedVolume);
	sound->setFilepath(filepath);
	if (!sound->load())
	{
		delete sound;
		sound = nullptr;
		return;
	}

	m_sounds.insert(std::make_pair(identifier, sound));
}

void Audiosystem::createMusic(std::string identifier, std::string filepath)
{
	auto it = m_music.find(identifier);
	if (it != m_music.end())
	{
		std::cout << "Failed to create music. Music '" << filepath << "' does already exist." << std::endl;
		return;
	}

	float myModifiedVolume = static_cast<float>(m_musicVolume)* m_masterVolume;

	Music* music = new Music();
	music->setVolume(myModifiedVolume);
	if (!music->openFromFile(filepath))
	{
		delete music;
		music = nullptr;
		return;
	}

	m_music.insert(std::make_pair(identifier, music));
}

void Audiosystem::changeMasterVolume(float masterVolume)
{
	m_masterVolume = clampf(masterVolume, 0.f, 1.f);
	changeSoundVolume(m_soundVolume);
	changeMusicVolume(m_musicVolume);
}

void Audiosystem::changeSoundVolume(int volume)
{
	m_soundVolume = clamp(volume, 0, 100);
	float myModifiedVolume = static_cast<float>(m_soundVolume) * m_masterVolume;

	auto it = m_sounds.begin();
	while (it != m_sounds.end())
	{
		it->second->setVolume(static_cast<int>(myModifiedVolume));
		++it;
	}
}

void Audiosystem::changeMusicVolume(int volume)
{
	m_musicVolume = clamp(volume, 0, 100);
	float myModifiedVolume = static_cast<float>(m_musicVolume) * m_masterVolume;

	auto it = m_music.begin();
	while (it != m_music.end())
	{
		it->second->setVolume(static_cast<int>(myModifiedVolume));
		++it;
	}
}

float Audiosystem::getMasterVolume()
{
	return m_masterVolume;
}

int Audiosystem::getRealSoundVolume()
{
	return static_cast<int>(m_masterVolume * static_cast<float>(m_soundVolume));
}

int Audiosystem::getRealMusicVolume()
{
	return static_cast<int>(m_masterVolume * static_cast<float>(m_musicVolume));
}

int Audiosystem::getSoundVolume()
{
	return m_soundVolume;
}

int Audiosystem::getMusicVolume()
{
	return m_musicVolume;
}

void Audiosystem::playSound(std::string identifier)
{
	auto it = m_sounds.find(identifier);
	if (it == m_sounds.end())
	{
		std::cout << "Failed to play sound. Sound '" << identifier << "' was not found." << std::endl;
		return;
	}

	it->second->play();
}

void Audiosystem::playMusic(std::string identifier)
{
	auto it = m_music.find(identifier);
	if (it == m_music.end())
	{
		std::cout << "Failed to play music. Music '" << identifier << "' was not found." << std::endl;
		return;
	}

	it->second->play();
}

void Audiosystem::update()
{
	auto it = m_sounds.begin();
	while (it != m_sounds.begin())
	{
		it->second->update();
		++it;
	}
}

void Sound::update()
{
	for (int i = 0; i < m_soundInstances.size(); ++i)
	{
		if (m_soundInstances[i].getStatus() == sf::Sound::Stopped)
		{
			m_soundInstances.erase(m_soundInstances.begin() + i);
			--i;
		}
	}
}

void Sound::setVolume(int volume)
{
	m_volume = volume;
}

int Sound::getVolume()
{
	return m_volume;
}

void Sound::play()
{
	sf::Sound sound(m_soundBuffer);
	m_soundInstances.push_back(sound);
	m_soundInstances.back().play();
}

bool Sound::load()
{
	if (!m_soundBuffer.loadFromFile(m_filepath))
	{
		return false;
	}
	return true;
}

void Sound::setFilepath(std::string filepath)
{
	m_filepath = filepath;
}

std::string Sound::getFilepath()
{
	return m_filepath;
}

Music::Music()
{

}

Music::~Music()
{

}
