#pragma once
#include <string>
#include <map>
#include <deque>
#include "SFML\Audio\SoundBufferRecorder.hpp"
#include "SFML\Audio\Music.hpp"
#include "SFML\Audio\Sound.hpp"

class Sound;
class Music;

class Audiosystem
{
public:
	Audiosystem(float masterVolume = 1.f, int soundVolume = 100, int musicVolume = 100);
	~Audiosystem();

	void createSound(std::string identifier, std::string filepath);
	void createMusic(std::string identifier, std::string filepath);

	void changeMasterVolume(float masterVolume);
	void changeSoundVolume(int volume);
	void changeMusicVolume(int volume);
	float getMasterVolume();
	int getRealSoundVolume();
	int getRealMusicVolume();
	int getSoundVolume();
	int getMusicVolume();

	void playSound(std::string identifier);
	void playMusic(std::string identifier);
	
	void update();

private:
	float m_masterVolume;
	int m_soundVolume;
	int m_musicVolume;
	
	std::map<std::string, Sound*> m_sounds;
	std::map<std::string, Music*> m_music;
};

class Sound
{
public:
	void update();
	void play();
	bool load();
	void setVolume(int volume);
	int getVolume();
	void setFilepath(std::string filepath);
	std::string getFilepath();
private:
	int m_volume;
	std::string m_filepath;
	std::deque<sf::Sound> m_soundInstances;
	sf::SoundBuffer m_soundBuffer;
};

class Music: public sf::Music
{
public:
	Music();
	~Music();
};