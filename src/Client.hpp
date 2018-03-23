#pragma once

#include <SFML/Network/TcpSocket.hpp>

#include <thread>
#include <mutex>
#include <queue>

struct PRIVMSG
{
	std::string username;
	std::string message;
	std::string nickname; // display name
	unsigned int color = 0;
};

class Client
{
public:
	bool connect(const std::string& filename);
	void disconnect();

	bool isAdmin(const std::string& username) const;
	// TODO: bool isModerator(const std::string& username) const;

	bool isMessageQueueEmpty() const;
	PRIVMSG popMessage();

	void sendPRIVMSG(const std::string& msg);

private:
	PRIVMSG parsePRIVMSG(const std::string& msg);

	bool send(const std::string& data);
	void receive();

private:
	sf::TcpSocket m_socket;
	std::string m_channel;
	std::thread m_thread;
	mutable std::mutex m_mutex;
	std::queue<PRIVMSG> m_messageQueue;
};
