#include "Client.hpp"
#include "Utility.hpp"

#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

bool Client::connect(const std::string& filename)
{
	std::string pass;
	std::string nick;

	if (std::ifstream ifs(filename); !ifs || !std::getline(ifs, pass) || !std::getline(ifs, nick) || !std::getline(ifs, m_channel))
	{
		std::cout << "[Client] Failed to load config file!\n";
		return false;
	}

	if (m_socket.connect("irc.chat.twitch.tv", 6667) != sf::Socket::Done)
	{
		std::cout << "[Client] Failed to connect to server!\n";
		return false;
	}

	send("PASS " + pass + "\r\n");
	send("NICK " + nick + "\r\n");
	send("JOIN " + m_channel + "\r\n");

	send("CAP REQ :twitch.tv/membership\r\n");
	send("CAP REQ :twitch.tv/tags\r\n");
	send("CAP REQ :twitch.tv/commands\r\n");

	m_thread = std::thread(&Client::receive, this);

	return true;
}

void Client::disconnect()
{
	m_socket.disconnect();
	m_thread.join();
}

bool Client::isAdmin(const std::string& username) const
{
	return username == m_channel.substr(1);
}

bool Client::isMessageQueueEmpty() const
{
	std::lock_guard<std::mutex> lock(m_mutex);

	return m_messageQueue.empty();
}

PRIVMSG Client::popMessage()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	PRIVMSG priv = m_messageQueue.front();
	m_messageQueue.pop();

	return priv;
}

void Client::sendPRIVMSG(const std::string& msg)
{
	send("PRIVMSG " + m_channel + " :" + msg + "\r\n");
}

PRIVMSG Client::parsePRIVMSG(const std::string& msg)
{
	PRIVMSG priv;

	priv.username = getSubstring(msg, ":", "!");
	priv.message = getSubstring(msg, ".tmi.twitch.tv ", " ");

	if (priv.message == "PRIVMSG")
		priv.message = getSubstring(msg, m_channel + " :");

	priv.nickname = getSubstring(msg, "display-name=", ";");

	if (priv.nickname.empty())
		priv.nickname = priv.username;

	if (const std::string str = getSubstring(msg, "color=#", ";"); !str.empty())
		priv.color = std::stoi("0x" + str, 0, 16);

	return priv;
}

bool Client::send(const std::string& data)
{
	const std::string utf8 = ansiToUtf8(data);

	if (m_socket.send(utf8.data(), utf8.size()) != sf::Socket::Done)
	{
		std::cout << "[Client] Failed to send data!\n";
		return false;
	}

	#ifdef _DEBUG
	if (!startsWith(data, "PASS"))
		std::cout << "> " << data;
	#endif

	return true;
}

void Client::receive()
{
	char data[512];
	std::size_t received;

	while (m_socket.receive(data, sizeof(data), received) == sf::Socket::Done)
	{
		const std::string msg(data, received);

		#ifdef _DEBUG
		std::cout << "< " << msg;
		#endif

		if (msg == "PING :tmi.twitch.tv\r\n")
			send("PONG :tmi.twitch.tv\r\n");

		else
		{
			std::istringstream iss(utf8ToAnsi(msg));
			std::string str;

			while (std::getline(iss, str))
			{
				if (str.find("PRIVMSG") != std::string::npos ||
					str.find("JOIN") != std::string::npos ||
					str.find("PART") != std::string::npos)
				{
					if (str.back() == '\r')
						str.pop_back();

					std::lock_guard<std::mutex> lock(m_mutex);

					m_messageQueue.emplace(parsePRIVMSG(str));

					#ifndef _DEBUG
					const PRIVMSG& priv = m_messageQueue.back();
					std::cout << "[Twitch] " << priv.nickname << " " << priv.message << '\n';
					#endif
				}
			}
		}
	}
}
