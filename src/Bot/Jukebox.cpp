#include "Jukebox.hpp"
#include "../Client.hpp"
#include "../Utility.hpp"

#include <Windows.h>

#include <iostream>
#include <filesystem>
#include <random>
#include <array>

namespace fs = std::filesystem;

Jukebox::Jukebox(Client& client)
	: Bot(client)
{
	m_downloadThread = std::thread(&Jukebox::download, this);
	m_playThread = std::thread(&Jukebox::play, this);
}

Jukebox::~Jukebox()
{
	m_running = false;
	skipSong();

	m_condition.notify_all();
	m_downloadThread.join();
	m_playThread.join();
}

void Jukebox::handleEvent(const sf::Event& event)
{
}

void Jukebox::update(sf::Time dt)
{
}

void Jukebox::draw(sf::RenderTarget& target)
{
}

void Jukebox::handlePRIVMSG(const PRIVMSG& priv)
{
	const auto [first, second] = splitCommand(priv.message);

	if (first == "!currentsong" || first == "!cs")
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_currentSongName.empty())
			sendPRIVMSG('@' + priv.username + ' ' + m_currentSongName);
	}

	else if (first == "!prevsong" || first == "!ps")
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (!m_previousSongName.empty())
			sendPRIVMSG('@' + priv.username + ' ' + m_previousSongName);
	}

	// TODO: !nextsong

	else if (first == "!songrequest" || first == "!sr")
	{
		if (!second.empty())
		{
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				m_requests.emplace(priv.username, second);
			}

			m_condition.notify_all();
		}

		else
			sendPRIVMSG('@' + priv.username + " Usage: !sr [a link or song title] (Supported sites: https://rg3.github.io/youtube-dl/supportedsites.html)");
	}

	else if (first == "!skip")
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_state == State::PlayingDefaultSong)
		{
			skipSong();
			sendPRIVMSG('@' + priv.username + " Skipped the current song.");
		}

		else
			sendPRIVMSG('@' + priv.username + " Requested songs can't be skipped.");
	}

	// TODO: !volume, !pause
}

void Jukebox::download()
{
	while (m_running)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		if (m_requests.empty())
			m_condition.wait(lock, [this] { return !m_requests.empty() || !m_running; });

		else
		{
			const auto [username, link] = m_requests.front();
			std::wstring utf16Link = ansiToUtf16(link);
			m_requests.pop();

			lock.unlock();

			// TODO: limit song length (--get-duration)
			std::wstring cmd = L".\\Jukebox\\youtube-dl.exe --get-title --no-playlist --default-search \"ytsearch\" \"" + utf16Link + L"\"";
			std::string title  = exec(cmd);

			if (title.empty() || title.find("ERROR:") != std::string::npos)
			{
				sendPRIVMSG('@' + username + " Unrecognized request: " + link);
				continue;
			}

			else
			{
				if (title.back() == '\n')
					title.pop_back();

				sendPRIVMSG('@' + username + " Added song: " + title);
			}

			cmd = L".\\Jukebox\\youtube-dl.exe --no-playlist --default-search \"ytsearch\" -o \".\\Jukebox\\Download\\" + ansiToUtf16(username) + L" - %(id)s.%(ext)s\" \"" + utf16Link + L"\"";
			std::string request = exec(cmd);

			std::string filename = getSubstring(request, "[download] Destination: ", "\n");

			if (filename.empty())
				filename = getSubstring(request, "[download] ", " has already been downloaded");

			if (!filename.empty())
			{
				lock.lock();

				m_requestedSongs.emplace(filename, title);

				if (m_state == State::PlayingDefaultSong)
					skipSong();

				lock.unlock();
				m_condition.notify_one();
			}

			else
			{
				sendPRIVMSG('@' + username + " Unable to download the song");

				std::cout << "[Jukebox] Youtube-dl begin\n" << request << "[Jukebox] Youtube-dl end\n";
			}
		}
	}
}

void Jukebox::play()
{
	while (m_running)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		if (m_defaultSongs.empty() && m_requestedSongs.empty())
		{
			loadSongs(L"Jukebox/Music");
			shuffleSongs();

			if (m_defaultSongs.empty())
				m_condition.wait(lock, [this] { return !m_requestedSongs.empty() || !m_running; });
		}

		else
		{
			std::wstring filename;
			m_previousSongName = std::move(m_currentSongName);

			if (!m_requestedSongs.empty())
			{
				// std::tie(filename, m_currentSongName) = m_requestedSongs.front();
				filename = ansiToUtf16(m_requestedSongs.front().first);
				m_currentSongName = m_requestedSongs.front().second;
				m_requestedSongs.pop();
				m_state = State::PlayingRequestedSong;
			}

			else
			{
				filename = m_defaultSongs.front();
				m_defaultSongs.pop_front();
				m_state = State::PlayingDefaultSong;
			}

			lock.unlock();

			// NOTE: mpv keyboard control
			// https://mpv.io/manual/master/#keyboard-control
			// https://github.com/mpv-player/mpv/blob/master/etc/input.conf

			const std::wstring request = L".\\Jukebox\\mpv.com --no-video --volume " + std::to_wstring(m_volume) + L" \"" + filename + L"\"";
			playSong(request);

			// TODO: UTF16?
			// https://stackoverflow.com/questions/19193429/why-are-certain-unicode-characters-causing-stdwcout-to-fail-in-a-console-app

			if (std::cout.fail())
				std::cout.clear();

			lock.lock();

			m_state = State::DoingNothing;
		}
	}
}

void Jukebox::loadSongs(const std::wstring& directory)
{
	for (const auto& entry : fs::recursive_directory_iterator(directory))
	{
		if (fs::is_regular_file(entry))
		{
			const auto& ext = entry.path().extension();

			if (ext == ".mp3" || ext == ".mp4" || ext == ".ogg" || ext == ".flac" || ext == ".webm")
				m_defaultSongs.emplace_back(entry.path().wstring());
		}

		else if (!fs::is_directory(entry))
			std::cout << "[Jukebox] Unrecognized file: " << entry.path().string() << '\n';
	}

	std::cout << "[Jukebox] Total " << m_defaultSongs.size() << " songs added to the queue.\n";
}

void Jukebox::shuffleSongs()
{
	static std::random_device rd;
	static std::mt19937 mt(rd());

	std::shuffle(m_defaultSongs.begin(), m_defaultSongs.end(), mt);
}

void Jukebox::playSong(const std::wstring& cmd)
{
	std::shared_ptr<FILE> pipe(_wpopen(cmd.data(), L"r"), _pclose);

	if (!pipe)
		throw std::runtime_error("_popen() failed!");

	std::array<char, 128> buffer;
	std::string result;

	std::string playing;
	std::string artist;
	std::string title;

	while (!feof(pipe.get()))
	{
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();

		if (const std::size_t newline = result.find('\n'); newline != std::string::npos)
		{
			if (playing.empty())
			{
				playing = getSubstring(result, "Playing: ", "\n");

				if (!playing.empty() && m_currentSongName.empty())
				{
					playing = utf8ToAnsi(playing);

					const std::size_t fileBegin = playing.find_last_of("/\\");
					const std::size_t dirBegin = playing.find_last_of("/\\", fileBegin - 1);
					const std::size_t extBegin = playing.find_last_of('.');

					const std::string directory = playing.substr(dirBegin + 1, fileBegin - (dirBegin + 1));
					const std::string filename = playing.substr(fileBegin + 1, extBegin - (fileBegin + 1));

					std::lock_guard<std::mutex> lock(m_mutex);

					m_currentSongName = directory + " - " + filename;

					std::cout << "[Jukebox] Playing: " << m_currentSongName << '\n';
				}
			}

			if (artist.empty())
			{
				artist = getSubstring(result, "Artist: ", "\n");

				if (!artist.empty())
				{
					artist = utf8ToAnsi(artist);

					std::lock_guard<std::mutex> lock(m_mutex);

					m_currentSongName = artist + " - " + m_currentSongName;

					std::cout << "[Jukebox] Artist: " << artist << '\n';
				}
			}

			if (title.empty())
			{
				title = getSubstring(result, "Title: ", "\n");

				if (!title.empty())
				{
					title = utf8ToAnsi(title);

					std::lock_guard<std::mutex> lock(m_mutex);

					if (artist.empty())
						m_currentSongName = title;
					else
						m_currentSongName = artist + " - " + title;

					std::cout << "[Jukebox] Title: " << title << '\n';
				}
			}

			result = result.substr(newline + 1);
		}
	}
}

void Jukebox::skipSong()
{
	PostMessage(GetConsoleWindow(), WM_CHAR, 'q', 0);
}
