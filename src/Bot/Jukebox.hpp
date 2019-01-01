#pragma once

#include "../Bot.hpp"

#include <queue>
#include <thread>
#include <mutex>

class Jukebox : public Bot
{
public:
	enum class State
	{
		DoingNothing,
		PlayingDefaultSong,
		PlayingRequestedSong,
	};

public:
	explicit Jukebox(Client& client);
	~Jukebox();

	void handleEvent(const sf::Event& event) override;
	void update(sf::Time dt) override;
	void draw(sf::RenderTarget& target) override;

	void handlePRIVMSG(const PRIVMSG& priv) override;

private:
	void download();
	void play();

	void loadSongs(const std::wstring& directory);
	void shuffleSongs();

	void playSong(const std::wstring& cmd);
	void skipSong();

private:
	bool m_running = true;
	int m_volume = 75;
	State m_state = State::DoingNothing;
	std::deque<std::wstring> m_defaultSongs;
	std::queue<std::pair<std::string, std::string>> m_requestedSongs;
	std::queue<std::pair<std::string, std::string>> m_requests;
	std::string m_currentSongName;
	std::string m_previousSongName;
	std::thread m_downloadThread;
	std::thread m_playThread;
	std::mutex m_mutex;
	std::condition_variable m_condition;
};
